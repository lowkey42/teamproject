#include "highscore_manager.hpp"

namespace lux {

	Highscore_manager::Highscore_manager(){
		_bodies_to_parse = std::vector<util::rest::Http_body>();
		_highscores = std::unordered_map<std::string, std::vector<Highscore_list>>();
	}

	void Highscore_manager::_update(Time delta_time){

	}

}
