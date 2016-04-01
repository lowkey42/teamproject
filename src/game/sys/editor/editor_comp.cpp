#include "editor_comp.hpp"

#include <core/utils/sf2_glm.hpp>

namespace lux {
namespace sys {
namespace editor {

	using namespace unit_literals;

	void Editor_comp::load(sf2::JsonDeserializer& state,
	                       asset::Asset_manager& asset_mgr) {
		auto bounds = remove_units(_bounds);

		state.read_virtual(
			sf2::vmember("bounds", bounds)
		);

		_bounds = bounds * 1_m;
	}
	void Editor_comp::save(sf2::JsonSerializer& state)const {

		state.write_virtual(
			sf2::vmember("bounds", remove_units(_bounds))
		);
	}

}
}
}
