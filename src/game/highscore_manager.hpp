/** The Highscore-Manger for loading and publishing highscores ***************
 *                                                                           *
 * Copyright (c) 2016 Sebastian Schalow                                      *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

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
			Highscore_manager(asset::Asset_manager& manager);

			// Methods
			auto get_highscore(std::vector<std::string> levels) -> std::vector<util::maybe<asset::Ptr<Highscore_list>>>;
			void push_highscore(Highscore& Highscore);

			void update(Time delta_time);

		private:
			std::unordered_map<std::string, util::rest::Http_body> _bodies; // <level_id, Http_body>
			asset::Asset_manager& _assets;

			// Methods
			void parse_highscore(std::string level_id, std::string& content);

	};

}
