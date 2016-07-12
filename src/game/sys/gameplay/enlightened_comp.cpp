#include "enlightened_comp.hpp"

#include <core/utils/sf2_glm.hpp>


namespace lux {
namespace sys {
namespace gameplay {

	using namespace unit_literals;

	sf2_enumDef(Enlightened_State, disabled, pending, canceling, activating, enabled)

	void Enlightened_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager&) {
		auto air_time = _air_time / 1_s;
		auto final_booster_left = _final_booster_left / 1_s;

		state.read_virtual(
			sf2::vmember("color", _color),
			sf2::vmember("direction", _direction),
			sf2::vmember("velocity", _velocity),
			sf2::vmember("air_transformations", _air_transformations),
			sf2::vmember("radius", _radius),
			sf2::vmember("final_booster_time", _final_booster_time),
			sf2::vmember("latency_compensation", _latency_compensation),
			sf2::vmember("max_air_time", _max_air_time),
			sf2::vmember("smash_force", _smash_force),

			sf2::vmember("was_light", _was_light),
			sf2::vmember("state", _state),
			sf2::vmember("direction", _direction),
			sf2::vmember("air_transforms_left", _air_transforms_left),
			sf2::vmember("air_time", air_time),
			sf2::vmember("final_booster_left", final_booster_left),
			sf2::vmember("smashed", _smashed),
			sf2::vmember("smash", _smash)
		);

		_air_time = air_time * 1_s;
		_final_booster_left = final_booster_left * 1_s;
	}

	void Enlightened_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual(
			sf2::vmember("color", _color),
			sf2::vmember("direction", _direction),
			sf2::vmember("velocity", _velocity),
			sf2::vmember("air_transformations", _air_transformations),
			sf2::vmember("radius", _radius),
			sf2::vmember("final_booster_time", _final_booster_time),
			sf2::vmember("latency_compensation", _latency_compensation),
			sf2::vmember("max_air_time", _max_air_time),
			sf2::vmember("smash_force", _smash_force),

			sf2::vmember("was_light", _was_light),
			sf2::vmember("state", _state),
			sf2::vmember("direction", _direction),
			sf2::vmember("air_transforms_left", _air_transforms_left),
			sf2::vmember("air_time", _air_time/1_s),
			sf2::vmember("final_booster_left", _final_booster_left/1_s),
			sf2::vmember("smashed", _smashed),
			sf2::vmember("smash", _smash)
		);
	}

	Enlightened_comp::Enlightened_comp(ecs::Entity& owner) : Component(owner) {
	}

}
}
}
