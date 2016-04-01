#include "sprite_comp.hpp"

#include <core/utils/sf2_glm.hpp>

#include <sf2/sf2.hpp>

#include <string>

namespace lux {
namespace sys {
namespace graphic {

	using namespace unit_literals;

	void Sprite_comp::load(sf2::JsonDeserializer& state,
	                       asset::Asset_manager& assets){
		std::string aid = _material ? _material.aid().str() : "";

		state.read_virtual(
			sf2::vmember("material", aid),
			sf2::vmember("size", _size),
			sf2::vmember("shadowcaster", _shadowcaster)
		);

		_material = assets.load<renderer::Material>(asset::AID(aid));
		INVARIANT(_material, "Material '"<<aid<<"' not found");
	}

	void Sprite_comp::save(sf2::JsonSerializer& state)const {
		std::string aid = _material ? _material.aid().str() : "";

		state.write_virtual(
			sf2::vmember("material", aid),
			sf2::vmember("size", _size),
			sf2::vmember("shadowcaster", _shadowcaster)
		);
	}

}
}
}
