#include "ai_patrolling_comp.hpp"

#include <core/ecs/serializer.hpp>
#include <core/utils/sf2_glm.hpp>

namespace lux {
namespace sys {
namespace controller {

	void load_component(ecs::Deserializer& state, Ai_patrolling_comp& comp) {
		state.read_virtual(
			sf2::vmember("velocity", comp._velocity),
			sf2::vmember("max_distance", comp._max_distance),
			sf2::vmember("flip_horizontal_on_return", comp._flip_horizontal_on_return),
			sf2::vmember("flip_vertical_on_return", comp._flip_vertical_on_return)
		);
	}

	void save_component(ecs::Serializer& state, const Ai_patrolling_comp& comp) {
		state.write_virtual(
			sf2::vmember("velocity", comp._velocity),
			sf2::vmember("max_distance", comp._max_distance),
			sf2::vmember("flip_horizontal_on_return", comp._flip_horizontal_on_return),
			sf2::vmember("flip_vertical_on_return", comp._flip_vertical_on_return)
		);
	}

}
}
}
