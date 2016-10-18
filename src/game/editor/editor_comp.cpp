#include "editor_comp.hpp"

#include <core/ecs/serializer.hpp>

#include <core/utils/sf2_glm.hpp>

namespace lux {
namespace editor {

	using namespace unit_literals;

	void load_component(ecs::Deserializer& state, Editor_comp& comp) {
		auto bounds = remove_units(comp._bounds);

		state.read_virtual(
			sf2::vmember("bounds", bounds)
		);

		comp._bounds = bounds * 1_m;
	}
	void save_component(ecs::Serializer& state, const Editor_comp& comp) {
		state.write_virtual(
			sf2::vmember("bounds", remove_units(comp._bounds))
		);
	}

}
}
