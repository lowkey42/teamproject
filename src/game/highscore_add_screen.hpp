/** Screen that is shown after each level, to add an entry to the highscore **
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "game_engine.hpp"
#include "highscore_manager.hpp"

#include <core/renderer/camera.hpp>
#include <core/renderer/texture.hpp>
#include <core/renderer/shader.hpp>
#include <core/renderer/vertex_object.hpp>
#include <core/renderer/primitives.hpp>
#include <core/renderer/text.hpp>
#include <core/renderer/command_queue.hpp>


namespace lux {

	class Highscore_add_screen : public Screen {
		public:
			Highscore_add_screen(Engine& engine, std::string level, Time time);
			~Highscore_add_screen()noexcept = default;

		protected:
			void _update(Time delta_time)override;
			void _draw()override;

			void _on_enter(util::maybe<Screen&> prev) override;
			void _on_leave(util::maybe<Screen&> next) override;

			auto _prev_screen_policy()const noexcept -> Prev_screen_policy override {
				return Prev_screen_policy::stack;
			}

		private:
			util::Mailbox_collection _mailbox;
			Highscore_manager& _highscores;

			renderer::Camera_2d _camera;

			renderer::Text_dynamic _debug_Text;

			renderer::Command_queue _render_queue;

			Highscore_list_ptr _highscore_list;

			const std::string _level_id;
			const Time _time;
			std::string _player_name;
	};

}
