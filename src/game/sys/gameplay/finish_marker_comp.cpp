#include "finish_marker_comp.hpp"


namespace lux {
namespace sys {
namespace gameplay {

	void Finish_marker_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager& asset_mgr) {
		state.read_virtual(
			sf2::vmember("required_color", _required_color),
			sf2::vmember("contained_colors", _contained_colors)
		);
	}

	void Finish_marker_comp::save(sf2::JsonSerializer& state) const {
		state.write_virtual(
			sf2::vmember("required_color", _required_color),
			sf2::vmember("contained_colors", _contained_colors)
		);
	}
}
}
}
