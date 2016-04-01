#include "rest.hpp"

#include "log.hpp"

#include <sstream>
#include <iomanip>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#else
#include <happyhttp/happyhttp.h>
#endif


namespace lux {
namespace util {
namespace rest {

	namespace {
		void append_url_encoded(std::stringstream& stream, const std::string str) {
			for(auto c : str) {
				if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
					stream << c;
				} else {
					stream << std::uppercase;
					stream << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
					stream << std::nouppercase;
				}
			}
		}
		auto build_param_str(const Http_parameters& params) -> std::string {
			std::stringstream param_str;
			param_str.fill('0');
			param_str << std::hex;

			if(!params.empty()) {
				bool first = true;
				for(auto& param : params) {
					if(first) first = false;
					else param_str << "&";

					append_url_encoded(param_str, param.first);
					param_str << "=";
					append_url_encoded(param_str, param.second);
				}
			}

			return param_str.str();
		}
		auto build_path(const std::string path, const Http_parameters& params) -> std::string {
			std::stringstream path_with_params;
			path_with_params<<path;

			if(!params.empty()) {
				path_with_params<<"?";
				path_with_params<<build_param_str(params);
			}

			return path_with_params.str();
		}
	}


#ifdef EMSCRIPTEN

	namespace {
		struct Request {
			std::string url;
			std::promise<std::string> promise;

			Request(std::string url) : url(std::move(url)) {}
		};

		std::vector<std::unique_ptr<Request>> pending_requests;

		auto exec_req(const char* method, const std::string& host, int port, const std::string& path,
		              const Http_parameters& get_param,
		              const Http_parameters& post_param) -> Http_body {

			std::stringstream url;
			url<<"http://"<<host<<":"<<port<<build_path(path, get_param);

			auto req = std::make_unique<Request>(url.str());

			auto post_str = build_param_str(post_param);

			emscripten_async_wget2_data(req->url.c_str(), method, post_str.c_str(),
			                            req.get(), true, +[](unsigned int, void* arg, void* data, unsigned int len) {

				auto req  = static_cast<Request*>(arg);
				auto resp = std::string((char*)data, len);

				req->promise.set_value(resp);
				pending_requests.erase(std::remove_if(pending_requests.begin(), pending_requests.end(),
				                                      [&](auto& a){
					return a.get()==req;
				}), pending_requests.end());

			}, +[](unsigned int, void* arg, int code, const char* error) {
				auto req  = static_cast<Request*>(arg);
				req->promise.set_value("");
				WARN("HTTP request to \""<<req->url<< "\" failed with "<<code<<": "<<error);
				pending_requests.erase(std::remove_if(pending_requests.begin(), pending_requests.end(),
				                                      [&](auto& a){
					return a.get()==req;
				}), pending_requests.end());
			},  +[](unsigned int, void*,int,int){});

			auto response = req->promise.get_future();
			pending_requests.emplace_back(std::move(req));
			return response;
		}
	}

	void update() {
	}

	auto post_request(const std::string& host, int port, const std::string& path,
	                  const Http_parameters& get_param,
	                  const Http_parameters& post_param) -> Http_body {
		return exec_req("POST", host, port, path, get_param, post_param);
	}

	auto get_request(const std::string& host, int port, const std::string& path,
	                 const Http_parameters& get_param) -> Http_body {
		return exec_req("GET", host, port, path, get_param, Http_parameters{});
	}

#else

	namespace {
		struct Request {
			happyhttp::Connection connection;
			std::promise<std::string> promise;
			std::stringstream body_buffer;

			Request(happyhttp::Connection c) : connection(std::move(c)) {}
		};

		void on_content(const happyhttp::Response* r, void* userdata, const unsigned char* data, int n) {
			auto req = static_cast<Request*>(userdata);

			req->body_buffer.write(reinterpret_cast<const char*>(data), n);
		}
		void on_complete(const happyhttp::Response* r, void* userdata) {
			auto req = static_cast<Request*>(userdata);

			req->promise.set_value(req->body_buffer.str());
		}

		std::vector<std::unique_ptr<Request>> open_connections;

		auto exec_req(const char* method, const char* host, int port,
		              const char* path, const std::string& post) -> Http_body {

			try {
				auto req = std::make_unique<Request>(happyhttp::Connection{host, port});

				req->connection.setcallbacks(
						+[](const happyhttp::Response*, void*){}, //< on_begin
						on_content,
						on_complete,
						req.get() );

				req->connection.request(method, path, nullptr,
				                        reinterpret_cast<const unsigned char*>(post.data()), post.size());

				auto response = req->promise.get_future();

				open_connections.emplace_back(std::move(req));

				return response;

			} catch(happyhttp::Wobbly e) {
				WARN("HTTP request to \"http://"<<host<<":"<<port<<path<< "\" failed: "<<e.what());
				std::promise<std::string> dummy;
				auto future = dummy.get_future();
				dummy.set_value("");
				return future;
			}
		}
	}

	void update() {
		for(auto iter=open_connections.begin(); iter!=open_connections.end();) {
			try {
				if((*iter)->connection.outstanding()) {
					(*iter)->connection.pump();
					iter++;
				} else {
					iter = open_connections.erase(iter);
				}
			} catch(happyhttp::Wobbly e) {
				iter = open_connections.erase(iter);
			}
		}
	}

	auto post_request(const std::string& host, int port, const std::string& path,
	                  const Http_parameters& get_param,
	                  const Http_parameters& post_param) -> Http_body {
		return exec_req("POST", host.c_str(), port, build_path(path, get_param).c_str(),
		                build_param_str(post_param));
	}

	auto get_request(const std::string& host, int port, const std::string& path,
	                 const Http_parameters& get_param) -> Http_body {
		return exec_req("GET", host.c_str(), port, build_path(path, get_param).c_str(), "");
	}

#endif


	auto get_body(Http_body& f) -> maybe<std::string> {
		if(f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			return just(f.get());
		} else {
			return nothing();
		}
	}

}
}
}
