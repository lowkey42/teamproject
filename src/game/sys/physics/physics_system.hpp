/** LiquidFun integration ****************************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "physics_comp.hpp"

#include <core/utils/maybe.hpp>
#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


class b2World;

namespace lux {
namespace sys {
namespace physics {

	struct Raycast_result {
		glm::vec2 normal;
		float distance;
	};

	class Physics_system {
		public:
			Physics_system(Engine&, ecs::Entity_manager&);
			~Physics_system();

			void update(Time);

			void update_body_shape(Dynamic_body_comp&);
			void update_body_shape(Static_body_comp&);

			auto raycast(glm::vec2 position, glm::vec2 dir, float max_dist) -> util::maybe<Raycast_result>;
			auto raycast(glm::vec2 position, glm::vec2 dir, float max_dist,
			             ecs::Entity& exclude) -> util::maybe<Raycast_result>;

		private:
			Dynamic_body_comp::Pool& _bodies_dynamic;
			Static_body_comp::Pool& _bodies_static;

			std::unique_ptr<b2World> _world;
			float _dt_acc = 0.f;
			// TODO

			void _get_positions();
			void _set_positions(float alpha);
	};

}
}
}
