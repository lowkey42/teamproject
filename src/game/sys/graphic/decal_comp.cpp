#define BUILD_SERIALIZER

#include "decal_comp.hpp"

#include <core/utils/sf2_glm.hpp>

#include <sf2/sf2.hpp>

#include <string>

namespace lux {
namespace sys {
namespace graphic {

	using namespace unit_literals;

	void Decal_comp::load(sf2::JsonDeserializer& state,
	                      asset::Asset_manager& assets) {
		std::string aid = _texture ? _texture.aid().str() : "";

		state.read_virtual(
			sf2::vmember("texture", aid),
			sf2::vmember("size", _size)
		);

		if(!aid.empty())
			_texture = assets.load<renderer::Texture>(asset::AID(aid));
	}
	void Decal_comp::save(sf2::JsonSerializer& state)const {
		std::string aid = _texture ? _texture.aid().str() : "";

		state.write_virtual(
			sf2::vmember("material", aid),
			sf2::vmember("size", _size)
		);
	}

}
}
}
