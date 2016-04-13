#include "light_comp.hpp"

#include <core/utils/sf2_glm.hpp>

namespace lux {
namespace sys {
namespace light {

	using namespace unit_literals;

	namespace {
		constexpr auto light_cutoff = 0.02f;
	}

	void Light_comp::load(sf2::JsonDeserializer& state, asset::Asset_manager& asset_mgr) {
		auto direction = _direction / 1_deg;
		auto angle = _angle / 1_deg;
		auto radius = 0.f;

		state.read_virtual(
			sf2::vmember("direction", direction),
			sf2::vmember("angle", angle),
			sf2::vmember("color", _color),
			sf2::vmember("radius", radius),
			sf2::vmember("factors", _factors),
			sf2::vmember("offset", _offset)
		);

		if(_radius_based && glm::length2(_factors)<0.0001)
			radius = remove_unit(_radius);


		if(std::abs(radius)>0.001) {
			// based on radius
			_radius_based = true;

			float a = light_cutoff;
			float d = radius;
			float x = 1.f;
			float y = 0.f;
			float z = (1-a*(d*y + x))
			          / (a*d*d);

			_factors = glm::vec3{
				x,y,z
			};

		} else {
			// based on factors
			_radius_based = false;

			if(_factors.z<0.00001) {
				radius = 100.f;
			} else {
				float a = light_cutoff;
				float x = _factors.x;
				float y = _factors.y;
				float z = _factors.z;

				radius = (glm::sqrt(a*(a*(y*y - 4*x*z) + 4*z)) + a*y)
				         / (2*a*z);
			}
		}

		_direction = direction * 1_deg;
		_angle = angle * 1_deg;
		_radius = radius * 1_m;
	}

	void Light_comp::save(sf2::JsonSerializer& state)const {
		if(_radius_based) {
			state.write_virtual(
				sf2::vmember("direction", _direction / 1_deg),
				sf2::vmember("angle", _angle / 1_deg),
				sf2::vmember("color", _color),
				sf2::vmember("radius", remove_unit(_radius)),
				sf2::vmember("offset", _offset)
			);

		} else {
			state.write_virtual(
				sf2::vmember("direction", _direction / 1_deg),
				sf2::vmember("angle", _angle / 1_deg),
				sf2::vmember("color", _color),
				sf2::vmember("factors", _factors),
				sf2::vmember("offset", _offset)
			);
		}
	}


	Light_comp::Light_comp(ecs::Entity& owner)
	    : Component(owner), _direction(0_deg), _angle(360_deg), _color(1,1,1),
	      _factors(1,0,1), _radius_based(false),
	      _radius(glm::sqrt(1-light_cutoff)/glm::sqrt(light_cutoff)) {
	}

}
}
}
