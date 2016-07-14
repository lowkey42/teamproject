#pragma once

#include <string>
#include <vector>

#include <core/asset/asset_manager.hpp>

namespace lux{

	struct Highscore {

		std::string name;
		int32_t score;

	};

	struct Highscore_list {

		std::string level;
		std::vector<Highscore> scores;
		int64_t timestamp;

	};
}
