/** Intermediate screen, allowing it to discard all previous screens *********
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "game_screen.hpp"

#include <core/engine.hpp>

namespace lux {

	class Loading_screen : public Screen {
		public:
			Loading_screen(Engine& engine, const std::string& level_id)
			    : Screen(engine), _level_id(level_id) {}
			~Loading_screen()noexcept = default;

		protected:
			void _update(Time delta_time)override {
				if(_done) {
					_engine.screens().leave();
				} else {
					_engine.screens().enter<Game_screen>(_level_id);
					_done = true;
				}
			}
			void _draw()override{}

			auto _prev_screen_policy()const noexcept -> Prev_screen_policy override {
				return Prev_screen_policy::discard;
			}

		private:
			std::string _level_id;
			bool _done=false;
	};

}
