#include "sprite_comp.hpp"

#include <core/utils/sf2_glm.hpp>

#include <sf2/sf2.hpp>

#include <string>

namespace mo {
namespace sys {
namespace graphic {

	using namespace unit_literals;

	void Sprite_comp::load(sf2::JsonDeserializer& state,
	                       asset::Asset_manager& assets){
		std::string aid = _material ? _material.aid().str() : "";
		auto size = remove_units(_size);

		state.read_virtual(
			sf2::vmember("material", aid),
			sf2::vmember("size", size)
		);

		_material = assets.load<renderer::Material>(asset::AID(aid));
		INVARIANT(_material, "Material '"<<aid<<"' not found");
		_size = size * 1_m;
	}

	void Sprite_comp::save(sf2::JsonSerializer& state)const {
		std::string aid = _material ? _material.aid().str() : "";

		state.write_virtual(
			sf2::vmember("material", aid),
			sf2::vmember("size", remove_units(_size))
		);
	}

}
}
}
