/** The sidebar to create entities from blueprints ***************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "editor_comp.hpp"

#include <core/asset/asset_manager.hpp>

#include <core/input/input_manager.hpp>

#include <core/renderer/texture.hpp>
#include <core/renderer/camera.hpp>
#include <core/renderer/texture_batch.hpp>

#include <core/ecs/ecs.hpp>

#include <core/utils/command.hpp>
#include <core/utils/messagebus.hpp>


namespace lux {
	namespace renderer {
		class Command_queue;
	}

namespace editor {

	struct Editor_conf;
	struct Blueprint_group;
	class Selection;

	class Blueprint_bar {
		public:
			Blueprint_bar(Engine&, util::Command_manager& commands, Selection& selection,
			              ecs::Entity_manager& entity_manager, asset::Asset_manager& assets,
			              input::Input_manager& input_manager,
			              renderer::Camera& camera_world, renderer::Camera& camera_ui,
			              glm::vec2 offset);

			void draw(renderer::Command_queue& queue);
			void update();
			auto handle_pointer(util::maybe<glm::vec2> mp1,
			                    util::maybe<glm::vec2> mp2) -> bool; //< true = mouse-input has been used

			auto is_in_delete_zone(util::maybe<glm::vec2> mp1) -> bool;

		private:
			Engine&                             _engine;
			renderer::Camera&                   _camera_world;
			renderer::Camera&                   _camera_ui;
			glm::vec2                           _offset;
			util::Mailbox_collection            _mailbox;
			util::Command_manager&              _commands;
			Selection&                          _selection;
			ecs::Entity_manager&                _entity_manager;
			input::Input_manager&               _input_manager;
			renderer::Texture_ptr               _background;
			renderer::Texture_ptr               _back_button;
			std::shared_ptr<const Editor_conf>  _conf;
			util::maybe<const Blueprint_group&> _current_category = util::nothing();

			bool                     _mouse_pressed = false;
			glm::vec2                _last_mouse_pos;
			util::maybe<std::size_t> _dragging = util::nothing();

			mutable renderer::Texture_batch _batch;

			void _spawn_new(std::size_t index, glm::vec2 pos);
	};

}
}
