#include "ai_patrolling_comp.hpp"

#include <core/utils/sf2_glm.hpp>

namespace lux {
namespace sys {
namespace controller {

	void Ai_patrolling_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager&) {
		state.read_virtual(
			sf2::vmember("velocity", _velocity),
			sf2::vmember("max_distance", _max_distance),
			sf2::vmember("flip_horizontal_on_return", _flip_horizontal_on_return),
			sf2::vmember("flip_vertical_on_return", _flip_vertical_on_return)
		);
	}

	void Ai_patrolling_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual(
			sf2::vmember("velocity", _velocity),
			sf2::vmember("max_distance", _max_distance),
			sf2::vmember("flip_horizontal_on_return", _flip_horizontal_on_return),
			sf2::vmember("flip_vertical_on_return", _flip_vertical_on_return)
		);
	}

}
}
}
