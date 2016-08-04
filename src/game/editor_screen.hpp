/** The ingame level editor **************************************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "level.hpp"
#include "meta_system.hpp"

#include "editor/blueprint_bar.hpp"
#include "editor/selection.hpp"
#include "editor/menu_bar.hpp"

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

			// TODO: top-side buttons
			// TODO: save-file management
			// TODO: plot player movement trail
			// TODO: level settings

			renderer::Camera_2d _camera_menu;
			renderer::Camera_sidescroller _camera_world;

			renderer::Text_dynamic _debug_Text;

			renderer::Command_queue _render_queue;

			editor::Selection _selection;
			editor::Blueprint_bar _blueprints;
			editor::Menu_bar _menu;

			util::maybe<std::string> _clipboard;
			util::maybe<glm::vec2> _last_pointer_pos;
			bool _cam_mouse_active = false;
			glm::vec2 _cam_speed;
			util::maybe<float> _real_ambient_light = util::nothing();

			Level_info _level_metadata;

			auto _handle_pointer_cam(util::maybe<glm::vec2> mp1, util::maybe<glm::vec2> mp2) -> bool;
			void _load_next_level(int dir);
			bool _load_next_level_allowed();
	};

}
