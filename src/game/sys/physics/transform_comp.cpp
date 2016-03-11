#include "transform_comp.hpp"

#include <core/utils/sf2_glm.hpp>
#include <core/units.hpp>
#include <sf2/sf2.hpp>

namespace mo {
namespace sys {
namespace physics {

	using namespace unit_literals;

	void Transform_comp::load(sf2::JsonDeserializer& state,
	                          asset::Asset_manager&){
		auto position_f = remove_units(_position);
		auto rotation_f = _rotation / 1_deg;

		state.read_virtual(
			sf2::vmember("position", position_f),
			sf2::vmember("scale", _scale),
			sf2::vmember("rotation", rotation_f),
			sf2::vmember("rotation_fixed", _rotation_fixed)
		);

		_position = position_f * 1_m;
		_rotation = rotation_f * 1_deg;
		_dirty = State::uninitialized;
	}
	void Transform_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual(
			sf2::vmember("position", remove_units(_position)),
			sf2::vmember("scale", _scale),
			sf2::vmember("rotation", _rotation / 1_deg),
			sf2::vmember("rotation_fixed", _rotation_fixed)
		);
	}

	void Transform_comp::position(Position pos)noexcept {
		_position=pos;
		if(_dirty==State::clear)
			_dirty=State::dirty;
	}
	void Transform_comp::rotation(Angle a)noexcept {
		if(!_rotation_fixed)
			_rotation = a;
	}

}
}
}
