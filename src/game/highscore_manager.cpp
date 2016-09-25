#include "highscore_manager.hpp"

#include <core/utils/template_utils.hpp>

#ifdef _WIN32
	#include <windows.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <errno.h>
	#include <ctype.h>
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <pwd.h>
#endif

#include <chrono>
#include <sstream>

namespace lux {

	using namespace std::chrono;
	using namespace unit_literals;

	namespace {
		struct Config {
			std::string host;
			int port;
			std::string path;
		};
		sf2_structDef(Config, host, port, path)

		struct Local_config {
			std::string last_username;
		};
		sf2_structDef(Local_config, last_username)

		// Login information
		const std::string host = "localhost";
		const std::string path = "/databasePHP/database.php";
		constexpr int     port = 80;

		// Time-Delta for synchronizing entries with the database in minutes
		constexpr int TDELTA = 15;

		auto current_timestamp() {
			auto now = system_clock::now().time_since_epoch();
			return duration_cast<minutes>(now).count();
		}
		auto highscore_list_aid(const std::string& level_id) {
			return asset::AID("highscore"_strid, level_id);
		}
		auto valid(const Highscore_list& hlist) {
			return current_timestamp() - hlist.timestamp <= TDELTA;
		}
		auto valid_ptr(const Highscore_list_ptr& hlist) {
			return hlist && valid(*hlist);
		}

		auto determine_username() -> std::string;

#ifdef _WIN32
		std::string win_to_utf8(const wchar_t* wstr, DWORD len) {
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, (int)len, NULL, 0, NULL, NULL);

			std::string out(size_needed, 0);
			WideCharToMultiByte                  (CP_UTF8, 0, wstr, (int)len, &out[0], size_needed, NULL, NULL);
			return out;
		}

		auto determine_username() -> std::string {
			DWORD bufsize = 0;

			if(GetUserNameW(NULL, &bufsize) == 0) {
				wchar_t* wbuf = (wchar_t*) malloc(bufsize * sizeof(wchar_t));
				ON_EXIT {
					free(wbuf);
				};

				if(GetUserNameW(wbuf, &bufsize) != 0) {
					return win_to_utf8(wbuf, bufsize);
				}
			}

			return "???";
		}
#else
		auto determine_username() -> std::string {
			auto uid = getuid();
			auto pw = getpwuid(uid);
			if(pw && pw->pw_name) {
				return pw->pw_name;
			}

			auto env_user = getenv("USER");
			if(env_user) {
				return env_user;
			}

			return "???";
		}
#endif
	}

	int Highscore_list::get_rank(Time time)const {
		int i = 0;

		for(auto& score : scores) {
			if(score.time>time/1_s)
				break;

			i++;
		}

		return i;
	}


	Highscore_manager::Highscore_manager(asset::Asset_manager& m)
	    : _assets(m) {

		auto cfg = _assets.load_maybe<Config>("cfg:remote_highscore"_aid);
		if(cfg.is_some()) {
			_remote_host = cfg.get_or_throw()->host;
			_remote_port = cfg.get_or_throw()->port;
			_remote_path = cfg.get_or_throw()->path;
		} else {
			_remote_host = host;
			_remote_port = port;
			_remote_path = path;
		}

		auto local_cfg = _assets.load_maybe<Local_config>("cfg:local_highscore"_aid);
		if(local_cfg.is_some()) {
			_last_username = local_cfg.get_or_throw()->last_username;
		} else {
			_last_username = determine_username();
		}


		INFO("Highscore server: "<<_remote_host<<":"<<_remote_port<<"/"<<_remote_path);
	}


	Highscore_list_results Highscore_manager::get_highscores(const std::vector<std::string>& level_ids) {
		Highscore_list_results ret;
		ret.reserve(level_ids.size());

		// For each level_id entry inside the given vector:
		for(auto& level_id : level_ids) {
			ret.emplace_back(get_highscore(level_id));
		}

		return ret;
	}
	auto Highscore_manager::get_highscore(const std::string& level_id) -> Highscore_list_ptr {
		auto level_aid = highscore_list_aid(level_id);

		auto hlist = Highscore_list_ptr{_assets, level_aid};
		hlist.try_load(true, false);

		// fetch list from remote if not already in process
		if(!valid_ptr(hlist) && _running_requests.find(level_id)==_running_requests.end()) {
			// Create a new Http-Body-Object for fetching the Highscore information from the database
			auto params = util::rest::Http_parameters{{"level", level_id}, {"op", "ghigh"}};
			auto body = util::rest::get_request(_remote_host, _remote_port, _remote_path, params);
			_running_requests.emplace(level_id, std::move(body));
		}

		return hlist;
	}


	void Highscore_manager::push_highscore(std::string level_id, const Highscore& score) {
		if(_last_username != score.name) {
			_last_username = score.name;
			_assets.save("cfg:local_highscore"_aid, Local_config{_last_username});
		}

		// convert score-value from int to str
		auto time_str = util::to_string(score.time);

		// building the http-request
		auto post_params = util::rest::Http_parameters{
				{"name", score.name},
				{"level", level_id},
				{"time", time_str},
				{"op", "phigh"}
		};
		auto body = util::rest::post_request(_remote_host, _remote_port, _remote_path, {}, post_params);
		_post_requests.push_back(std::move(body));


		// check if a local copy of the level-highscores exists
		auto aid = highscore_list_aid(level_id);
		auto existing = _assets.load_maybe<Highscore_list>(aid, true, false).process(Highscore_list{}, [](auto& a){return *a;});

		// add score and reset timestamp to force re-fetch on next get_highscore
		existing.level = level_id;
		existing.timestamp = 0;
		existing.scores.emplace_back(score);
		std::sort(existing.scores.begin(), existing.scores.end(), [](auto& lhs, auto& rhs){return lhs.time<rhs.time;});

		_assets.save(aid, existing);
	}


	void Highscore_manager::update(Time delta_time){
		// not necessary, just prints out the POST-result
		for(auto it = _post_requests.begin(); it != _post_requests.end(); ) {
			auto content = util::rest::get_content(*it);
			if(content.is_some()){
				it = _post_requests.erase(it);
			} else {
				it++;
			}
		}

		// Check entries inside the map, if any of the Http-body-objects
		// got successfully fetched and start parsing-procedure afterwards
		for(auto it=_running_requests.begin(); it != _running_requests.end(); ) {
			auto content = util::rest::get_content(it->second);

			if(content.is_some()){
				_parse_highscore(it->first, content.get_or_throw());
				it = _running_requests.erase(it);
			} else {
				it++;
			}
		}

	}


	void Highscore_manager::_parse_highscore(std::string level_id, std::string& content) {
		Highscore_list list;
		list.level = level_id;

		list.timestamp = current_timestamp();

		// parse highscores from given content-string
		auto content_stream = std::istringstream{content};
		sf2::deserialize_json(content_stream, [&](auto& msg, uint32_t row, uint32_t column) {
			DEBUG("Error parsing highscore JSON response for "<<level_id<<" at "<<row<<":"<<column<<": "<<msg);
			DEBUG("BODY: "<<content);
		}, list);

		if(level_id!=list.level) {
			WARN("Compromised result from remote (level_id mismatch) for level: "<<level_id);
			return;
		}

		_assets.save(highscore_list_aid(level_id), list);
	}

}
