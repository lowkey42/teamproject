/** deprecated | Manages the highscore list (load, store, add) ***************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <string>
#include <vector>

namespace lux {
	namespace asset{class Asset_manager;}


	struct Score {
		std::string name;
		int32_t     score;
		int32_t     level;
		uint64_t    seed;
	};

	extern void add_score(asset::Asset_manager& assets, Score score);

	extern void prepare_list_scores(asset::Asset_manager& assets);

	extern auto list_scores(asset::Asset_manager& assets) -> std::vector<Score>;

	extern auto print_scores(std::vector<Score> scores) -> std::string;

}
