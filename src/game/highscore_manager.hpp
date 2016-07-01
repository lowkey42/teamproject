#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <core/utils/rest.hpp>
#include <core/utils/maybe.hpp>
#include <core/asset/asset_manager.hpp>

#include <sf2/sf2.hpp>

#include "../core/engine.hpp"
#include "highscore.hpp"

namespace lux {

	sf2_structDef(Highscore,
		name,
		score
	)

	sf2_structDef(Highscore_list,
		level,
		scores
	)

	class Highscore_manager	{

		public:
			// Constructors
			Highscore_manager();

			// Methods
			auto get_highscore(asset::Asset_manager &manager, std::vector<std::string> levels) -> std::vector<util::maybe<asset::Ptr<Highscore_list>>>;
			void push_highscore(Highscore& Highscore);

			void update(Time delta_time);

		private:
			std::unordered_map<std::string, util::rest::Http_body> _bodies; // <level_id, Http_body>
			// std::unordered_map<std::string, Highscore_list> _highscores;		// <level_id, Highscore_list>

			// Methods
			void parse_highscore(util::rest::Http_body& body);

	};

}
