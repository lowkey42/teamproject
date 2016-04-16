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
namespace controller {

	class Controller_system {
		public:
			Controller_system(Engine&, ecs::Entity_manager&);

			void update(Time);

		private:
			Input_controller_comp::Pool& _input_controllers;
			// TODO
	};

}
}
}
