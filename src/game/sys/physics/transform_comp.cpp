#include "transform_comp.hpp"

#include <core/ecs/serializer.hpp>
#include <core/utils/sf2_glm.hpp>
#include <core/units.hpp>
#include <sf2/sf2.hpp>


namespace lux {
namespace sys {
namespace physics {

	using namespace unit_literals;

	void load_component(ecs::Deserializer& state, Transform_comp& comp){
		auto position_f = remove_units(comp._position);
		auto rotation_f = comp._rotation / 1_deg;

		state.read_virtual(
			sf2::vmember("position", position_f),
			sf2::vmember("scale", comp._scale),
			sf2::vmember("rotation", rotation_f),
			sf2::vmember("rotation_fixed", comp._rotation_fixed),
			sf2::vmember("flip_horizontal", comp._flip_horizontal),
			sf2::vmember("flip_vertical", comp._flip_vertical)
		);

		comp._position = position_f * 1_m;
		comp._rotation = rotation_f * 1_deg;
	}
	void save_component(ecs::Serializer& state, const Transform_comp& comp) {
		state.write_virtual(
			sf2::vmember("position", remove_units(comp._position)),
			sf2::vmember("scale", comp._scale),
			sf2::vmember("rotation", comp._rotation / 1_deg),
			sf2::vmember("rotation_fixed", comp._rotation_fixed),
			sf2::vmember("flip_horizontal", comp._flip_horizontal),
			sf2::vmember("flip_vertical", comp._flip_vertical)
		);
	}

	void Transform_comp::position(Position pos)noexcept {
		_position=pos;
		_revision++;
	}
	void Transform_comp::rotation(Angle a)noexcept {
		if(!_rotation_fixed) {
			_rotation = a;
		}
	}
	void Transform_comp::flip_horizontal(bool f)noexcept {
		if(_flip_horizontal!=f) {
			_flip_horizontal = f;
			_revision++;
		}
	}
	void Transform_comp::flip_vertical(bool f)noexcept {
		if(_flip_vertical!=f) {
			_flip_vertical = f;
			_revision++;
		}
	}

	auto Transform_comp::resolve_relative(glm::vec3 offset)const -> glm::vec3 {
		offset.x *= _scale;
		offset.y *= _scale;

		if(_flip_horizontal)
			offset.x *= -1.0;
		if(_flip_vertical)
			offset.y *= -1.0;

		auto xy = rotate(glm::vec2{offset.x, offset.y}, _rotation);

		return {xy.x, xy.y, offset.z};
	}

}
}
}
