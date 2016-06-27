#include "highscore_manager.hpp"

#include <sf2/sf2.hpp>

namespace lux {

	sf2_structDef(Highscore,
		_name,
		_score
	)

	sf2_structDef(Highscore_list,
		_level,
		_scores
	)

	Highscore_manager::Highscore_manager(){
		_bodies_to_parse = std::vector<util::rest::Http_body>();
		_highscores = std::unordered_map<std::string, Highscore_list>();
	}

	std::vector<util::maybe<asset::Ptr<Highscore_list>>> Highscore_manager::get_highscore(asset::Asset_manager &assets, std::vector<std::string> levels){

		asset::AID curAID = asset::AID(levels.at(0));

		std::vector<util::maybe<asset::Ptr<Highscore_list>>> vec;
		vec.push_back(assets.load_maybe<Highscore_list>(curAID));

		return vec;

	}

	void Highscore_manager::test(asset::Asset_manager& assets, std::string level){

		asset::AID curLevel(level);

		/*assets.save(curLevel, Highscore{"temp", 42});
		WARN("highscore saved");*/

		util::maybe<asset::Ptr<Highscore>> tmp = assets.load_maybe<Highscore>(curLevel);
		if(tmp.is_some()){
			WARN("highscore reloaded");
			tmp.process([&](asset::Ptr<Highscore> elem) {
				//WARN("Score value: " << elem->_score);
			});
		} else {
			WARN("highscore not loaded!");
		}



	}


	void Highscore_manager::update(Time delta_time){

	}

}
