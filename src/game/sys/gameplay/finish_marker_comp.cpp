#include "finish_marker_comp.hpp"

#include <core/ecs/serializer.hpp>


namespace lux {
namespace sys {
namespace gameplay {

	void load_component(ecs::Deserializer& state, Finish_marker_comp& comp) {
		state.read_virtual(
			sf2::vmember("required_color", comp._required_color),
			sf2::vmember("contained_colors", comp._contained_colors)
		);
	}

	void save_component(ecs::Serializer& state, const Finish_marker_comp& comp) {
		state.write_virtual(
			sf2::vmember("required_color", comp._required_color),
			sf2::vmember("contained_colors", comp._contained_colors)
		);
	}
}
}
}
