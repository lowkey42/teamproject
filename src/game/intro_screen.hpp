/**************************************************************************\
 * The intro screen that is shown when launching the game                 *
 *                                               ___                      *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2014 Florian Oetke                                       *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include "../core/engine.hpp"
#include <core/renderer/camera.hpp>
#include "../core/renderer/texture.hpp"
#include "../core/renderer/shader.hpp"
#include "../core/renderer/vertex_object.hpp"
#include "../core/renderer/primitives.hpp"
#include "../core/renderer/text.hpp"
#include <core/renderer/command_queue.hpp>

namespace mo {

	class Intro_screen : public Screen {
		public:
			Intro_screen(Engine& game_engine);
			~Intro_screen()noexcept = default;

		protected:
			void _update(Time delta_time)override;
			void _draw()override;

			void _on_enter(util::maybe<Screen&> prev) override;
			void _on_leave(util::maybe<Screen&> next) override;

			auto _prev_screen_policy()const noexcept -> Prev_screen_policy override {
				return Prev_screen_policy::discard;
			}

		private:
			util::Mailbox_collection _mailbox;

			renderer::Camera _camera;

			renderer::Text_dynamic _debug_Text;

			renderer::Command_queue _render_queue;
	};

}
