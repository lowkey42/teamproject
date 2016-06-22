#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <core/utils/rest.hpp>

#include "../core/engine.hpp"
#include "highscore.hpp"

namespace lux {

	class Highscore_manager	{

		public:
			// Constructors
			Highscore_manager();

			// Methods
			void fetch_all_scores();	 // initial when starting the game
			void fetch_level_scores(std::string level); // Called if new highscore has been pushed / level is created and saved
			void push_score(Highscore& Highscore);

			auto show_highscores(std::string level) -> util::maybe<std::vector<Highscore&>>;
			auto show_global_scores() -> util::maybe<std::vector<Highscore&>>;

			void parse_highscore();

		protected:
			void _update(Time delta_time);

		private:
			std::unordered_map<std::string, Highscore> _highscores; // local storage of highscores
			util::rest::Http_body _http_body;
			bool _fetching; // set if _http_body is still to be fetched


	};

}
