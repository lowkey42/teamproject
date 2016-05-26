#include "ai_simple_comp.hpp"

namespace lux {
namespace sys {
namespace controller {

	void Ai_simple_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager&) {
		state.read_virtual(
			sf2::vmember("velocity", _velocity)
		);
	}

	void Ai_simple_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual(
			sf2::vmember("velocity", _velocity)
		);
	}

}
}
}
