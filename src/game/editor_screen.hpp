/**************************************************************************\
 * The ingame level editor                                                *
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

#include "meta_system.hpp"

#include "sys/editor/editor_system.hpp"

#include <core/engine.hpp>
#include <core/renderer/camera.hpp>
#include <core/renderer/texture.hpp>
#include <core/renderer/shader.hpp>
#include <core/renderer/vertex_object.hpp>
#include <core/renderer/primitives.hpp>
#include <core/renderer/text.hpp>
#include <core/renderer/command_queue.hpp>

namespace mo {

	class Editor_screen : public Screen {
		public:
			Editor_screen(Engine& game_engine);
			~Editor_screen()noexcept = default;

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

			Meta_system _systems;
			sys::editor::Editor_system _editor_sys;

			// TODO: sidebar for blueprints
			// TODO: entity placement
			// TODO: camera movement
			// TODO: top-side buttons
			// TODO: load/store
			// TODO: save-file management
			// TODO: entity manipulation (move, rotate, scale)
			// TODO: entity deletion
			// TODO: play/pause functionallity
			// TODO: plot player movement trail
			// TODO: level settings
			// TODO: entity settings
			// TODO: smart textures

			renderer::Camera _camera_menu;
			renderer::Camera _camera_world;

			renderer::Text_dynamic _debug_Text;

			renderer::Command_queue _render_queue;

			ecs::Entity_ptr _selected_entity;

			void _on_drag(glm::vec2 src, glm::vec2 target);
	};

}
