/** System managing all gameplay components **********************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "enlightened_comp.hpp"

#include <core/renderer/camera.hpp>
#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


namespace lux {
namespace sys {
namespace cam {
	class Camera_system;
}
namespace physics {
	class Dynamic_body_comp;
	class Physics_system;
}
namespace gameplay {

	class Gameplay_system {
		public:
			Gameplay_system(Engine&, ecs::Entity_manager&, physics::Physics_system& physics_world,
			                cam::Camera_system& camera_sys);

			void update(Time);

		private:
			util::Mailbox_collection _mailbox;
			Enlightened_comp::Pool& _enlightened;
			physics::Physics_system& _physics_world;
			cam::Camera_system& _camera_sys;

			Time _light_timer{0};
			// TODO
	};

}
}
}
