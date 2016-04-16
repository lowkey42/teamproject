#include "physics_system.hpp"

namespace lux {
namespace sys {
namespace physics {

	Physics_system::Physics_system(Engine&, ecs::Entity_manager& ecs)
	    : _bodies(ecs.list<Physics_comp>()) {
		// TODO
	}

	void Physics_system::update(Time) {
		// TODO
	}

}
}
}
