/** A graph prividing structured access to all entites in the scene **********
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <functional>
#include <vector>
#include <array>
#include <glm/gtx/norm.hpp>

#include <core/utils/maybe.hpp>

#include "../../../core/utils/template_utils.hpp"
#include "transform_comp.hpp"


namespace lux {
namespace sys {
namespace physics {

	class Transform_system {
		public:
			Transform_system(ecs::Entity_manager& entity_manager);

			void update(Time dt){}
	};

	using Scene_graph = Transform_system;

}
}
}
