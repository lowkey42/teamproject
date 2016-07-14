#include "highscore_manager.hpp"

#include <chrono>
#include <sstream>

namespace lux {

	using namespace std::chrono;

	sf2_structDef(Highscore,
		name,
		score
	)

	sf2_structDef(Highscore_list,
		level,
		scores,
		timestamp
	)

	namespace {
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
	}


	Highscore_manager::Highscore_manager(asset::Asset_manager& m) : _assets(m){}


	Highscore_list_results Highscore_manager::get_highscore(const std::vector<std::string>& level_ids) {
		Highscore_list_results ret;
		ret.reserve(level_ids.size());

		// For each level_id entry inside the given vector:
		for(auto& level_id : level_ids) {
			auto level_aid = highscore_list_aid(level_id);
			auto hlist = _assets.load_maybe<Highscore_list>(level_aid);

			if(!hlist.process(false, &valid_ptr))
				hlist = util::nothing();

			ret.push_back(hlist);

			// fetch list from remote if not already in process
			if(hlist.is_nothing() && _running_requests.find(level_id)==_running_requests.end()) {
				// Create a new Http-Body-Object for fetching the Highscore information from the database
				auto params = util::rest::Http_parameters{{"level", level_id}, {"op", "ghigh"}};
				auto body = util::rest::get_request(host, port, path, params);
				_running_requests.emplace(level_id, std::move(body));
			}
		}

		return ret;
	}


	void Highscore_manager::push_highscore(std::string level_id, const Highscore& score){
		// convert score-value from int to str
		auto score_str = util::to_string(score.score);

		// building the http-request
		auto post_params = util::rest::Http_parameters{
				{"name", score.name},
				{"level", level_id},
				{"score", score_str},
				{"op", "phigh"}
		};
		auto body = util::rest::post_request(host, port, path, {}, post_params);
		_post_requests.push_back(std::move(body));


		// check if a local copy of the level-highscores exists
		auto aid = highscore_list_aid(level_id);
		auto existing = _assets.load_maybe<Highscore_list>(aid);

		existing.process([&](asset::Ptr<Highscore_list> hlist) {
			// add score and reset timestamp to force re-fetch on next get_highscore
			auto cpy = *hlist;
			cpy.timestamp = 0;
			cpy.scores.emplace_back(score);

			_assets.save(aid, cpy);
		});
	}


	void Highscore_manager::update(Time delta_time){
		// not necessary, just prints out the POST-result
		for(auto it = _post_requests.begin(); it != _post_requests.end(); ) {
			auto content = util::rest::get_content(*it);
			if(content.is_some()){
				it = _post_requests.erase(it);
				content.process([&](auto str){
					DEBUG("POST result: " + str);
				});
			} else {
				it++;
			}
		}

		// Check entries inside the map, if any of the Http-body-objects
		// got successfully fetched and start parsing-procedure afterwards
		for(auto it=_running_requests.begin(); it != _running_requests.end(); ) {
			auto content = util::rest::get_content(it->second);

			if(content.is_some()){
				content.process([&](std::string& body){
					_parse_highscore(it->first, body);
				});
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
		auto reader = sf2::JsonDeserializer{content_stream, [&](auto& msg, uint32_t row, uint32_t column) {
			ERROR("Error parsing highscore JSON response for "<<level_id<<" at "<<row<<":"<<column<<": "<<msg);
		}};
		reader.read_value(list.scores);


		_assets.save(highscore_list_aid(level_id), list);
	}

}
