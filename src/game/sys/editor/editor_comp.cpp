#include "editor_comp.hpp"

#include <core/utils/sf2_glm.hpp>

namespace mo {
namespace sys {
namespace editor {

	void Editor_comp::load(sf2::JsonDeserializer& state,
	                       asset::Asset_manager& asset_mgr) {

		state.read_virtual(
			sf2::vmember("bounds", remove_units(_bounds))
		);
	}
	void Editor_comp::save(sf2::JsonSerializer& state)const {

		state.write_virtual(
			sf2::vmember("bounds", remove_units(_bounds))
		);
	}

}
}
}
