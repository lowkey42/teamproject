#include "input_controller_comp.hpp"

#include <core/ecs/serializer.hpp>


namespace lux {
namespace sys {
namespace controller {

	void load_component(ecs::Deserializer& state, Input_controller_comp& comp) {
		state.read_virtual(
			sf2::vmember("air_velocity", comp._air_velocity),
			sf2::vmember("ground_velocity", comp._ground_velocity),
			sf2::vmember("acceleration_time", comp._acceleration_time),
			sf2::vmember("jump_velocity", comp._jump_velocity),
			sf2::vmember("jump_cooldown", comp._jump_cooldown)
		);
	}

	void save_component(ecs::Serializer& state, const Input_controller_comp& comp) {
		state.write_virtual(
			sf2::vmember("air_velocity", comp._air_velocity),
			sf2::vmember("ground_velocity", comp._ground_velocity),
			sf2::vmember("acceleration_time", comp._acceleration_time),
			sf2::vmember("jump_velocity", comp._jump_velocity),
			sf2::vmember("jump_cooldown", comp._jump_cooldown)
		);
	}

	Input_controller_comp::Input_controller_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
	    : Component(manager, owner) {
	}

}
}
}
