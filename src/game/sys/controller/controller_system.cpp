#include "controller_system.hpp"

namespace lux {
namespace sys {
namespace controller {

	Controller_system::Controller_system(Engine& engine, ecs::Entity_manager& ecs)
	    : _input_controllers(ecs.list<Input_controller_comp>()) {
		// TODO
	}

	void Controller_system::update(Time) {
		// TODO
	}

}
}
}
