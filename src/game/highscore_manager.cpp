#include "highscore_manager.hpp"

#include <chrono>
#include <sstream>

namespace lux {

	// Login information
	std::string host = "localhost";
	int port = 80;
	std::string path = "/databasePHP/database.php";

	// Time-Delta for synchronizing entries with the database in minutes
	const int TDELTA = 30;


	Highscore_manager::Highscore_manager(asset::Asset_manager& m) : _assets(m){}


	std::vector<util::maybe<asset::Ptr<Highscore_list>>> Highscore_manager::get_highscore(std::vector<std::string> level_ids){

		std::vector<util::maybe<asset::Ptr<Highscore_list>>> ret;

		// For each level_id entry do the following
		for(auto& level_id : level_ids) {

			auto level_aid = asset::AID("highscore:" + level_id);
			util::maybe<asset::Ptr<Highscore_list>> hlist = _assets.load_maybe<Highscore_list>(level_aid);

			// Check if questioned Highscore-List could be loaded by Asset-Manager
			if(hlist.is_some()){
				bool cont = false;
				hlist.process([&](asset::Ptr<Highscore_list> reference){
					// Check if difference between timestamp in hlist and cur-Time is lower than TDELTA
					int64_t curMins = 0;
					{
						using namespace std::chrono;
						minutes mins = duration_cast<minutes>(system_clock::now().time_since_epoch());
						curMins = mins.count();
					}
					if((curMins - reference->timestamp) < TDELTA){
						ret.push_back(reference);
						cont = true;
						DEBUG("Found Highscore-List " << level_id << " and placed it inside return-vector");
					}
				});
				if(cont)
					continue;
			}

			WARN("Highscore-List " << level_id << " couldn't be loaded / timestamp delta to high");
			std::unordered_map<std::string, util::rest::Http_body>::iterator it = _bodies.find(level_id);

			// Check if a fetching Http-Body allready exists for questioned Highscore-List
			if(it != _bodies.end()){
				DEBUG("HTTP-Body-Object fetching the level " << level_id << " allready exists in map!");
				continue;
			}

			// Create a new Http-Body-Object for fetching the Highscore information from the database
			WARN("HTTP-Body-Object fetching the level " << level_id << " doesn't exist");
			util::rest::Http_parameters params {{"level", level_id}, {"op", "ghigh"}};
			util::rest::Http_body body = util::rest::get_request(host, port, path, params);
			_bodies.emplace(level_id, std::move(body));
			DEBUG("Placed " << level_id << " corresponding HTTP-Body-Object inside the map!");

		}

		return ret;

	}


	void Highscore_manager::push_highscore(std::string level_id, Highscore& score){

		// convert score-value from int to str
		std::stringstream sstr;
		sstr << score.score;

		// building the http-request
		util::rest::Http_parameters post_params {{"name", score.name}, {"level", level_id}, {"score", sstr.str()}, {"op", "phigh"}};
		util::rest::Http_body body = util::rest::post_request(host, port, path, post_params, {});
		_post_requests.push_back(std::move(body));


		// check if a local copy of the level-highscores exists
		asset::AID level_aid("highscore:" + level_id);
		util::maybe<asset::Ptr<Highscore_list>> maybe = _assets.load_maybe<Highscore_list>(level_aid);
		if(maybe.is_some()){
			maybe.process([&](asset::Ptr<Highscore_list> hlist){
				// set timestamp to 0 so that next time highscore-list is requested
				// it is forced to load it from the server instead
				// (posting new highscores changes hlist for corresponding level)
				Highscore_list newList {hlist->level, hlist->scores, 0};
				_assets.save(level_aid, newList);
			});
		}

		DEBUG("pushed " + level_id + " to server!");



	}


	void Highscore_manager::update(Time delta_time){

		for(auto it = _post_requests.begin(); it != _post_requests.end(); ){

			auto content = util::rest::get_content(*it);
			if(content.is_some()){
				it = _post_requests.erase(it);
				content.process([&](auto str){
					DEBUG("Push result: " + str);
				});
			} else {
				it++;
			}

		}

		// Check entries inside the map, if any of the Http-body-objetcs got succesfully fetched
		for(auto it=_bodies.begin(); it != _bodies.end(); ){

			auto content = util::rest::get_content(it->second);
			if(content.is_some()){

				content.process([&](std::string& body){
					parse_highscore(it->first, body);
				});
				it = _bodies.erase(it);

			} else {
				it++;
			}

			/*
			WARN("Checking Http-Body-object of " << kv.first);
			util::maybe<std::string> maybe = util::rest::get_content(_bodies[kv.first]);
			if(maybe.is_some()){
				maybe.process([&](std::string content) {
					par
				});
				it = _bodies.find(kv.first);
				if(it != _bodies.end()){
					DEBUG(kv.first << " succesfully fetched, body removed!");
					_bodies.erase(it);
				}*/

		}
	}


	void Highscore_manager::parse_highscore(std::string level_id, std::string& content){

		Highscore_list list;
		list.level = level_id;

		{
			using namespace std::chrono;

			minutes mins = duration_cast<minutes>(system_clock::now().time_since_epoch());
			list.timestamp = mins.count();

		}

		// Begin parsing highscores from given content-string
		for(size_t i = 0; i < content.size(); i++){

			// Found a new highscore-entry
			if(content.at(i) == '['){

				Highscore score;
				size_t j = i + 1;
				do {

					// parsing entry-name
					if(content.substr(j, 9) == "\"name\": \""){

						size_t begin, end;
						begin = end = j + 9;

						while(content.at(end) != '"'){
							end++;
						}
						std::string name = content.substr(begin, end - begin);
						score.name = name;

					// parsing entry-value
					} else if (content.substr(j, 9) == "\"score\": "){

						size_t begin, end;
						begin = end = j + 9;

						while(content.at(end) != ']'){
							end++;
						}
						int val = std::atoi(content.substr(begin, end - begin).c_str());
						score.score = val;

					}

				} while(content.at(j++) != ']');

				if(score.name != "" && score.score > 0)
					list.scores.push_back(score);

			}

		}

		// parsed all highscore-entries
		DEBUG("Entries in Highscore-List for " << list.level);
		for(auto& highscore : list.scores){
			DEBUG(highscore.name << ": " << highscore.score << std::endl);
		}

		asset::AID saveAID("highscore:" + list.level);
		_assets.save(saveAID, list);

	}

}
