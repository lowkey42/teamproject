/** LiquidFun integration ****************************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "physics_comp.hpp"

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


namespace lux {
namespace sys {
namespace physics {

	class Physics_system {
		public:
			Physics_system(Engine&, ecs::Entity_manager&);

			void update(Time);

		private:
			Physics_comp::Pool& _bodies;
			// TODO
	};

}
}
}
