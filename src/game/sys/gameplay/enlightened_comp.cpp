#include "enlightened_comp.hpp"

#include <core/utils/sf2_glm.hpp>


namespace lux {
namespace sys {
namespace gameplay {


	void Enlightened_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager&) {
		state.read_virtual(
			sf2::vmember("direction", _direction),
			sf2::vmember("velocity", _velocity),
			sf2::vmember("air_transformations", _air_transformations),
			sf2::vmember("radius", _radius)
		);
	}

	void Enlightened_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual(
			sf2::vmember("direction", _direction),
			sf2::vmember("velocity", _velocity),
			sf2::vmember("air_transformations", _air_transformations),
			sf2::vmember("radius", _radius)
		);
	}

	Enlightened_comp::Enlightened_comp(ecs::Entity& owner) : Component(owner) {
	}

}
}
}