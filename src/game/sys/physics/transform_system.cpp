#include "transform_system.hpp"

#include <core/ecs/ecs.hpp>

#include<glm/gtc/constants.hpp>

namespace lux {
namespace sys {
namespace physics {

	Transform_system::Transform_system(
			ecs::Entity_manager& entity_manager) {

		entity_manager.register_component_type<physics::Transform_comp>();
	}

}
}
}
