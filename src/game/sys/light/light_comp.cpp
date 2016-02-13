#include "light_comp.hpp"

#include <core/utils/sf2_glm.hpp>

namespace mo {
namespace sys {
namespace light {

	using namespace unit_literals;

	void Light_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager& asset_mgr) {
		auto direction = _direction / 1_deg;
		auto angle = _angle / 1_deg;
		auto radius = remove_unit(_radius);


		state.read_virtual(
			sf2::vmember("direction", direction),
			sf2::vmember("angle", angle),
			sf2::vmember("color", _color),
			sf2::vmember("radius", radius),
			sf2::vmember("falloff", _falloff)
		);

		_direction = direction * 1_deg;
		_angle = angle * 1_deg;
		_radius = radius * 1_m;
	}

	void Light_comp::save(sf2::JsonSerializer& state)const {
		auto direction = _direction / 1_deg;
		auto angle = _angle / 1_deg;
		auto radius = remove_unit(_radius);


		state.write_virtual(
			sf2::vmember("direction", direction),
			sf2::vmember("angle", angle),
			sf2::vmember("color", _color),
			sf2::vmember("radius", radius),
			sf2::vmember("falloff", _falloff)
		);
	}

}
}
}
