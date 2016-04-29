/** System managing all gameplay components **********************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "enlightened_comp.hpp"
#include "player_tag_comp.hpp"

#include <core/renderer/camera.hpp>
#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>

#include <functional>


namespace lux {
	namespace renderer {
		class Command_queue;
		class Camera;
	}

namespace sys {
	namespace cam {
		class Camera_system;
	}
	namespace controller {
		class Controller_system;
	}
	namespace physics {
		class Dynamic_body_comp;
		class Physics_system;
	}

namespace gameplay {

	class Gameplay_system {
		public:
			Gameplay_system(Engine&, ecs::Entity_manager&, physics::Physics_system& physics_world,
			                cam::Camera_system& camera_sys,
			                controller::Controller_system& controller_sys,
			                std::function<void()> reload);

			void update(Time);
			void draw(renderer::Command_queue&, const renderer::Camera& camera)const;

		private:
			struct Blood_stain {
				glm::vec2 position;
				Blood_stain() = default;
				Blood_stain(glm::vec2 p) : position(p) {}
			};

			Engine& _engine;
			util::Mailbox_collection _mailbox;
			Enlightened_comp::Pool& _enlightened;
			Player_tag_comp::Pool& _players;
			physics::Physics_system& _physics_world;
			cam::Camera_system& _camera_sys;
			controller::Controller_system& _controller_sys;
			std::function<void()> _reload;

			Time _light_timer{0};

			std::vector<Blood_stain> _blood_stains;


			void _update_light(Time);
	};

}
}
}
