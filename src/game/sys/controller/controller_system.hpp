/** System controlling the actions of all entites ****************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "input_controller_comp.hpp"

#include <core/renderer/camera.hpp>
#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


namespace lux {
namespace sys {
namespace physics {
	class Dynamic_body_comp;
	class Physics_system;
}
namespace controller {

	class Controller_system {
		public:
			Controller_system(Engine&, ecs::Entity_manager&, physics::Physics_system& physics_world);

			void update(Time);

			bool input_active();

			void block_input(Time t) {
				_input_block_remainder = t;
			}

		private:
			util::Mailbox_collection _mailbox;
			Input_controller_comp::Pool& _input_controllers;
			physics::Physics_system& _physics_world;

			bool _mouse_look = true;
			float _move_dir = 0.f;
			int _move_left = 0;
			int _move_right = 0;
			bool _jump = false;
			glm::vec2 _target_dir;
			bool _transform_pending = false;
			bool _transform = false;
			bool _transform_canceled = false;

			Time _input_block_remainder {};

			static void _move(Input_controller_comp& c, physics::Dynamic_body_comp& body, float dir, Time dt);
			static void _start_jump(Input_controller_comp& c, physics::Dynamic_body_comp& body, Time dt);
			static void _stop_jump(Input_controller_comp& c, physics::Dynamic_body_comp& body, Time dt);
	};

}
}
}
