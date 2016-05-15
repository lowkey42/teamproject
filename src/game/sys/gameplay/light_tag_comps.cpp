#include "light_tag_comps.hpp"

#include "../physics/transform_comp.hpp"

#include <core/utils/sf2_glm.hpp>


namespace lux {
namespace sys {
namespace gameplay {

	using namespace unit_literals;

	void Reflective_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager&) {
		state.read_virtual(
			sf2::vmember("color", _color)
		);
	}
	void Reflective_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual(
			sf2::vmember("color", _color)
		);
	}

	void Transparent_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager&) {
		state.read_virtual(
			sf2::vmember("color", _color)
		);
	}
	void Transparent_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual(
			sf2::vmember("color", _color)
		);
	}

	void Lamp_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager&) {
		auto angle    = _angle.in_degrees();
		auto max_dist = _max_distance.value();
		auto rotation = _rotation.in_degrees();
		auto offset   = remove_units(_offset);

		state.read_virtual(
			sf2::vmember("color", _color),
			sf2::vmember("angle", angle),
			sf2::vmember("radius", max_dist),
			sf2::vmember("rotation", rotation),
			sf2::vmember("offset", offset)
		);

		_angle = Angle::from_degrees(angle);
		_rotation = Angle::from_degrees(rotation);
		_max_distance = max_dist * 1_m;
		_offset = offset * 1_m;
	}
	void Lamp_comp::save(sf2::JsonSerializer& state)const {
		auto angle    = _angle.in_degrees();
		auto max_dist = _max_distance.value();
		auto rotation = _rotation.in_degrees();
		auto offset   = remove_units(_offset);

		state.write_virtual(
			sf2::vmember("color", _color),
			sf2::vmember("angle", angle),
			sf2::vmember("radius", max_dist),
			sf2::vmember("rotation", rotation),
			sf2::vmember("offset", offset)
		);
	}

	// TODO: test me
	auto Lamp_comp::in_range(Position p)const -> bool {
		auto& transform = owner().get<physics::Transform_comp>().get_or_throw();
		auto my_pos = transform.position() + _offset;

		auto diff = p - my_pos;

		auto dist2 = glm::length(remove_units(diff));

		if(dist2 <= _max_distance.value()*_max_distance.value()) {
			auto dir = Angle{glm::atan(diff.x.value(), diff.y.value())};

			auto dir_diff = std::abs(normalize(_rotation - dir));
			if(dir_diff>180_deg)
				dir_diff = 360_deg - dir_diff;
			dir_diff = std::abs(dir_diff);

			return dir_diff <= _angle/2.f;
		}

		return false;
	}


}
}
}
