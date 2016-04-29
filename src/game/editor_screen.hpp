/** The ingame level editor **************************************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "level.hpp"
#include "meta_system.hpp"

#include "sys/editor/editor_system.hpp"
#include "sys/editor/selection.hpp"

#include <core/renderer/camera.hpp>
#include <core/renderer/texture.hpp>
#include <core/renderer/shader.hpp>
#include <core/renderer/vertex_object.hpp>
#include <core/renderer/primitives.hpp>
#include <core/renderer/text.hpp>
#include <core/renderer/command_queue.hpp>
#include <core/utils/command.hpp>
#include <core/engine.hpp>


namespace lux {

	class Editor_screen : public Screen {
		public:
			Editor_screen(Engine& game_engine, const std::string& level_id);
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
			util::Command_manager _commands;
			input::Input_manager& _input_manager;

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

			renderer::Camera_2d _camera_menu;
			renderer::Camera_sidescroller _camera_world;

			renderer::Text_dynamic _debug_Text;

			renderer::Command_queue _render_queue;

			sys::editor::Selection _selection;
			util::maybe<std::string> _clipboard;
			util::maybe<glm::vec2> _last_pointer_pos;

			glm::vec2 _cam_speed;

			Level_data _level_metadata;

			auto _handle_pointer_menu(util::maybe<glm::vec2> mp1, util::maybe<glm::vec2> mp2) -> bool;
			auto _handle_pointer_cam(util::maybe<glm::vec2> mp1, util::maybe<glm::vec2> mp2) -> bool;
	};

}
