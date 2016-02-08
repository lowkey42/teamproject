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
		std::string aid = _texture ? _texture.aid().str() : "";
		auto size = remove_units(_size);

		state.read_virtual(
			sf2::vmember("aid", aid),
			sf2::vmember("size", size)
		);

		_texture = assets.load<renderer::Texture>(asset::AID(aid));
		INVARIANT(_texture, "Texture '"<<aid<<"' not found");
		INVARIANT(_texture->width()>1 && _texture->width()<4096 &&
		          _texture->height()>1 && _texture->height()<4096, "Invalid size of texture '"<<aid<<"'");
		_size = size * 1_m;
	}

	void Sprite_comp::save(sf2::JsonSerializer& state)const {
		std::string aid = _texture ? _texture.aid().str() : "";

		state.write_virtual(
			sf2::vmember("aid", aid),
			sf2::vmember("size", remove_units(_size))
		);
	}

}
}
}
