#define GLM_SWIZZLE

#include "light_tag_comps.hpp"

#include "../physics/transform_comp.hpp"

#include <core/ecs/serializer.hpp>

#include <core/utils/sf2_glm.hpp>


namespace lux {
namespace sys {
namespace gameplay {

	using namespace unit_literals;

	void load_component(ecs::Deserializer& state, Reflective_comp& comp) {
		state.read_virtual(
			sf2::vmember("color", comp._color)
		);
	}
	void save_component(ecs::Deserializer& state, const Reflective_comp& comp) {
		state.write_virtual(
			sf2::vmember("color", comp._color)
		);
	}

	void load_component(ecs::Deserializer& state, Paint_comp& comp) {
		state.read_virtual(
			sf2::vmember("color", comp._color),
			sf2::vmember("radius", comp._radius)
		);
	}
	void save_component(ecs::Deserializer& state, const Paint_comp& comp) {
		state.write_virtual(
			sf2::vmember("color", comp._color),
			sf2::vmember("radius", comp._radius)
		);
	}

	void load_component(ecs::Deserializer& state, Transparent_comp& comp) {
		state.read_virtual(
			sf2::vmember("color", comp._color)
		);
	}
	void save_component(ecs::Deserializer& state, const Transparent_comp& comp) {
		state.write_virtual(
			sf2::vmember("color", comp._color)
		);
	}

	void load_component(ecs::Deserializer& state, Lamp_comp& comp) {
		auto angle    = comp._angle.in_degrees();
		auto max_dist = comp._max_distance.value();
		auto rotation = comp._rotation.in_degrees();
		auto offset   = remove_units(comp._offset);

		state.read_virtual(
			sf2::vmember("color", comp._color),
			sf2::vmember("angle", angle),
			sf2::vmember("radius", max_dist),
			sf2::vmember("rotation", rotation),
			sf2::vmember("offset", offset)
		);

		comp._angle = Angle::from_degrees(angle);
		comp._rotation = Angle::from_degrees(rotation);
		comp._max_distance = max_dist * 1_m;
		comp._offset = offset * 1_m;
	}
	void save_component(ecs::Deserializer& state, const Lamp_comp& comp) {
		auto angle    = comp._angle.in_degrees();
		auto max_dist = comp._max_distance.value();
		auto rotation = comp._rotation.in_degrees();
		auto offset   = remove_units(comp._offset);

		state.write_virtual(
			sf2::vmember("color", comp._color),
			sf2::vmember("angle", angle),
			sf2::vmember("radius", max_dist),
			sf2::vmember("rotation", rotation),
			sf2::vmember("offset", offset)
		);
	}

	auto Lamp_comp::in_range(Position p)const -> bool {
		auto& transform = owner().get<physics::Transform_comp>().get_or_throw();
		auto my_pos = transform.position() + _offset;

		auto diff = (p - my_pos).xy();

		auto dist2 = glm::length(remove_units(diff));

		if(dist2 <= _max_distance.value()*_max_distance.value()) {
			auto dir = Angle{glm::atan(diff.x.value(), diff.y.value())};

			auto dir_diff = std::abs(normalize(_rotation - dir));
			if(dir_diff>180_deg)
				dir_diff = 360_deg - dir_diff;
			dir_diff = std::abs(dir_diff);

			return true;//TODO: _angle.value()<0.f || dir_diff <= _angle/2.f;
		}

		return false;
	}


	void load_component(ecs::Deserializer& state, Prism_comp& comp) {
		auto offset_red = remove_units(comp._offset_red);
		auto offset_green = remove_units(comp._offset_green);
		auto offset_blue = remove_units(comp._offset_blue);

		state.read_virtual(
			sf2::vmember("offset_red", offset_red),
			sf2::vmember("offset_green", offset_green),
			sf2::vmember("offset_blue", offset_blue)
		);

		comp._offset_red = offset_red * 1_m;
		comp._offset_green = offset_green * 1_m;
		comp._offset_blue = offset_blue * 1_m;
	}
	void save_component(ecs::Deserializer& state, const Prism_comp& comp) {
		auto offset_red = remove_units(comp._offset_red);
		auto offset_green = remove_units(comp._offset_green);
		auto offset_blue = remove_units(comp._offset_blue);

		state.write_virtual(
			sf2::vmember("offset_red", offset_red),
			sf2::vmember("offset_green", offset_green),
			sf2::vmember("offset_blue", offset_blue)
		);
	}

}
}
}
