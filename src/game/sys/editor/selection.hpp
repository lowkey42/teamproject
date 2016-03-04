/**************************************************************************\
 * Handles the editor selection (drawing, input and actions)              *
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

#include "editor_comp.hpp"

#include <core/ecs/ecs.hpp>

#include <core/renderer/camera.hpp>
#include <core/renderer/texture.hpp>
#include <core/renderer/shader.hpp>
#include <core/renderer/vertex_object.hpp>
#include <core/renderer/texture_batch.hpp>

#include <core/utils/command.hpp>
#include <core/units.hpp>
#include <core/engine.hpp>


namespace mo {
namespace sys {
namespace editor {

	class Selection {
		public:
			Selection(Engine& engine, ecs::Entity_manager& entity_manager,
			          renderer::Camera& world_cam, util::Command_manager&);

			void draw(renderer::Command_queue& queue, renderer::Camera&);
			void update();
			auto handle_pointer(util::maybe<glm::vec2> mp1,
			                    util::maybe<glm::vec2> mp2) -> bool; //< true = mouse-input has been used

			auto copy_content()const -> std::string;

		public:
			util::Mailbox_collection _mailbox;
			renderer::Camera& _world_cam;
			util::Command_manager& _commands;
			input::Input_manager& _input_manager;
			Editor_comp::Pool& _editor_comps;

			renderer::Texture_batch _batch;

			renderer::Texture_ptr _icon_layer;
			renderer::Texture_ptr _icon_move;
			renderer::Texture_ptr _icon_rotate;
			renderer::Texture_ptr _icon_scale;

			ecs::Entity_ptr _selected_entity;

			util::maybe<glm::vec2> _last_primary_pointer_pos;
			util::maybe<glm::vec2> _last_secondary_pointer_pos;

			// all coordinates in screen space
			void _change_selection(glm::vec2 point);
			void _move(glm::vec2 offset);
			void _move_layer(float offset);
			void _rotate(glm::vec2 pivot, Angle offset);
			void _scale(float factor);
	};

}
}
}
