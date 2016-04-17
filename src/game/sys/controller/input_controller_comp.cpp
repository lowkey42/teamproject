#include "input_controller_comp.hpp"

namespace lux {
namespace sys {
namespace controller {

	void Input_controller_comp::load(sf2::JsonDeserializer& state,
	                                 asset::Asset_manager&) {
		state.read_virtual(
			sf2::vmember("move_force", _move_force),
			sf2::vmember("jump_force", _jump_force),
			sf2::vmember("max_speed", _max_speed)
		);
	}

	void Input_controller_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual(
			sf2::vmember("move_force", _move_force),
			sf2::vmember("jump_force", _jump_force),
			sf2::vmember("max_speed", _max_speed)
		);
	}

	Input_controller_comp::Input_controller_comp(ecs::Entity& owner) : Component(owner) {
	}

}
}
}
