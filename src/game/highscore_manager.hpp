#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <core/utils/rest.hpp>
#include <core/utils/maybe.hpp>
#include <core/asset/asset_manager.hpp>

#include "../core/engine.hpp"
#include "highscore.hpp"

namespace lux {

	class Highscore_manager	{

		public:
			// Constructors
			Highscore_manager();

			// Methods
			auto get_highscore(asset::Asset_manager &manager, std::vector<std::string> levels) -> std::vector<util::maybe<asset::Ptr<Highscore_list>>>;
			void push_highscore(Highscore& Highscore);

			void update(Time delta_time);

			void test(asset::Asset_manager& assets, std::string level);

		private:
			std::vector<util::rest::Http_body> _bodies_to_parse;
			std::unordered_map<std::string, Highscore_list> _highscores;

			// Methods
			auto parse_highscore() -> Highscore_list;

	};

}
