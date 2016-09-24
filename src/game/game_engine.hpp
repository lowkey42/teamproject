/** An engine containing game-specific manager instances *********************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/engine.hpp>
#include <core/utils/maybe.hpp>


namespace lux {

	class Highscore_manager;

	class Game_engine : public Engine {
		public:
			Game_engine(const std::string& title, int argc, char** argv, char** env);
			~Game_engine();

			auto& highscore_manager()noexcept {return *_highscore_manager;}

		protected:
			void _on_frame(Time) override;

		private:
			std::unique_ptr<Highscore_manager> _highscore_manager;
	};

}
