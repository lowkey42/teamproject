#include "input_controller_comp.hpp"

namespace lux {
namespace sys {
namespace controller {

	void Input_controller_comp::load(sf2::JsonDeserializer& state,
	                                 asset::Asset_manager&) {
		state.read_virtual(
			sf2::vmember("air_velocity", _air_velocity),
			sf2::vmember("ground_velocity", _ground_velocity),
			sf2::vmember("acceleration_time", _acceleration_time),
			sf2::vmember("jump_velocity", _jump_velocity),
			sf2::vmember("jump_cooldown", _jump_cooldown)
		);
	}

	void Input_controller_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual(
			sf2::vmember("air_velocity", _air_velocity),
			sf2::vmember("ground_velocity", _ground_velocity),
			sf2::vmember("acceleration_time", _acceleration_time),
			sf2::vmember("jump_velocity", _jump_velocity),
			sf2::vmember("jump_cooldown", _jump_cooldown)
		);
	}

	Input_controller_comp::Input_controller_comp(ecs::Entity& owner) : Component(owner) {
	}

}
}
}
