#include "highscore_manager.hpp"

#include <sf2/sf2.hpp>

namespace lux {

	// Login-Information
	std::string host = "localhost";
	int port = 80;
	std::string path = "/databasePHP/database.php";


	Highscore_manager::Highscore_manager(){	}


	std::vector<util::maybe<asset::Ptr<Highscore_list>>> Highscore_manager::get_highscore(asset::Asset_manager &assets, std::vector<std::string> level_ids){

		std::vector<util::maybe<asset::Ptr<Highscore_list>>> ret;

		// For each level_id entry do the following
		for(auto& level_id : level_ids) {

			auto level_aid = asset::AID("highscore:" + level_id);
			util::maybe<asset::Ptr<Highscore_list>> hlist = assets.load_maybe<Highscore_list>(level_aid);

			// Check if questioned Highscore-List could be loaded by Asset-Manager
			if(hlist.is_some()){
				hlist.process([&](auto reference){
					ret.push_back(reference);
				});
				DEBUG("Found Highscore-List " << level_id << " and placed it inside return-vector");
				continue;
			}

			WARN("Highscore-List " << level_id << " couldn't be loaded");
			std::unordered_map<std::string, util::rest::Http_body>::iterator it = _bodies.find(level_id);

			// Check if a fetching Http-Body allready exists for questioned Highscore-List
			if(it != _bodies.end()){
				DEBUG("HTTP-Body-Object fetching the level " << level_id << " allready exists in map!");
				continue;
			}

			// Create a new Http-Body-Object for fetching the Highscore information from the database
			WARN("HTTP-Body-Object fetching the level " << level_id << " doesn't exist");
			util::rest::Http_parameters params {{"level", level_id}};
			util::rest::Http_body body = util::rest::get_request(host, port, path, params);
			_bodies.emplace(level_id, std::move(body));
			DEBUG("Placed " << level_id << " corresponding HTTP-Body-Object inside the map!");

		}

		return ret;

	}


	void Highscore_manager::update(Time delta_time){

		// Check entries inside the map, if any of the Http-body-objetcs got succesfully fetched
		for(auto& kv : _bodies){

			WARN("Checking Http-Body-object of " << kv.first);
			util::rest::get_content(_bodies[kv.first]);

		}
	}


	void Highscore_manager::parse_highscore(util::rest::Http_body& body){

	}

}
