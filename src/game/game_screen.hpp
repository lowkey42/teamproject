/** The screen running the gameplay for a specific level *********************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "meta_system.hpp"

#include <core/renderer/camera.hpp>
#include <core/renderer/command_queue.hpp>
#include <core/renderer/text.hpp>
#include <core/engine.hpp>
#include <core/utils/maybe.hpp>


namespace lux {

	class Game_screen : public Screen {
		public:
			Game_screen(Engine& game_engine, const std::string& level_id);
			~Game_screen()noexcept = default;

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
			Meta_system _systems;

			renderer::Text_dynamic _ui_text;
			renderer::Camera_2d _camera_ui;
			renderer::Command_queue _render_queue;

			std::string _current_level;

			bool _fadeout = false;
			Time _fadeout_fadetimer {};
	};

}
