/** The Highscore-Manger for loading and publishing highscores ***************
 *                                                                           *
 * Copyright (c) 2016 Sebastian Schalow                                      *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/asset/asset_manager.hpp>
#include <core/utils/rest.hpp>
#include <core/utils/maybe.hpp>
#include <core/engine.hpp>

#include <vector>
#include <string>
#include <unordered_map>


namespace lux {

	struct Highscore {
		std::string name;
		int32_t score;
	};

	struct Highscore_list {
		std::string level;
		std::vector<Highscore> scores;
		int64_t timestamp;
	};

	using Highscore_list_ptr = asset::Ptr<Highscore_list>;
	using Highscore_list_results = std::vector<util::maybe<Highscore_list_ptr>>;


	class Highscore_manager	{
		public:
			Highscore_manager(asset::Asset_manager&);

			/// Get the Highscore-lists for given level_ids
			auto get_highscore(const std::vector<std::string>& levels) -> Highscore_list_results;

			/// Push a new highscore-entry for a given level_id to the server
			void push_highscore(std::string level_id, const Highscore&);

			void update(Time delta_time);

		private:
			std::vector<util::rest::Http_body> _post_requests;
			std::unordered_map<std::string, util::rest::Http_body> _running_requests;
			asset::Asset_manager& _assets;

			void _parse_highscore(std::string level_id, std::string& content);

	};

}
