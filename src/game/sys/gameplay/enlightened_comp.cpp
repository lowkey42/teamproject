#include <core/ecs/serializer.hpp>

#include <core/utils/sf2_glm.hpp>

#include "enlightened_comp.hpp"


namespace lux {
namespace sys {
namespace gameplay {

	using namespace unit_literals;

	sf2_enumDef(Enlightened_State, disabled, pending, canceling, activating, enabled)

	void load_component(ecs::Deserializer& state, Enlightened_comp& comp) {
		auto air_time = comp._air_time / 1_s;
		auto final_booster_left = comp._final_booster_left / 1_s;

		state.read_virtual(
			sf2::vmember("color", comp._color),
			sf2::vmember("direction", comp._direction),
			sf2::vmember("velocity", comp._velocity),
			sf2::vmember("air_transformations", comp._air_transformations),
			sf2::vmember("radius", comp._radius),
			sf2::vmember("final_booster_time", comp._final_booster_time),
			sf2::vmember("latency_compensation", comp._latency_compensation),
			sf2::vmember("max_air_time", comp._max_air_time),
			sf2::vmember("smash_force", comp._smash_force),
			sf2::vmember("initial_booster_time", comp._initial_booster_time),
			sf2::vmember("initial_booster_factor", comp._initial_booster_factor),
			sf2::vmember("initial_acceleration_time", comp._initial_acceleration_time),

			sf2::vmember("was_light", comp._was_light),
			sf2::vmember("state", comp._state),
			sf2::vmember("direction", comp._direction),
			sf2::vmember("air_transforms_left", comp._air_transforms_left),
			sf2::vmember("air_time", air_time),
			sf2::vmember("final_booster_left", final_booster_left),
			sf2::vmember("smashed", comp._smashed),
			sf2::vmember("smash", comp._smash)
		);

		comp._air_time = air_time * 1_s;
		comp._final_booster_left = final_booster_left * 1_s;
	}

	void save_component(ecs::Serializer& state, const Enlightened_comp& comp) {
		state.write_virtual(
			sf2::vmember("color", comp._color),
			sf2::vmember("direction", comp._direction),
			sf2::vmember("velocity", comp._velocity),
			sf2::vmember("air_transformations", comp._air_transformations),
			sf2::vmember("radius", comp._radius),
			sf2::vmember("final_booster_time", comp._final_booster_time),
			sf2::vmember("latency_compensation", comp._latency_compensation),
			sf2::vmember("max_air_time", comp._max_air_time),
			sf2::vmember("smash_force", comp._smash_force),
			sf2::vmember("initial_booster_time", comp._initial_booster_time),
			sf2::vmember("initial_booster_factor", comp._initial_booster_factor),
			sf2::vmember("initial_acceleration_time", comp._initial_acceleration_time),

			sf2::vmember("was_light", comp._was_light),
			sf2::vmember("state", comp._state),
			sf2::vmember("direction", comp._direction),
			sf2::vmember("air_transforms_left", comp._air_transforms_left),
			sf2::vmember("air_time", comp._air_time/1_s),
			sf2::vmember("final_booster_left", comp._final_booster_left/1_s),
			sf2::vmember("smashed", comp._smashed),
			sf2::vmember("smash", comp._smash)
		);
	}

	Enlightened_comp::Enlightened_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
	    : Component(manager, owner) {
	}

}
}
}
