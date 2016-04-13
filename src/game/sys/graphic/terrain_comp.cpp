#include "terrain_comp.hpp"

#include <core/utils/sf2_glm.hpp>

#include <sf2/sf2.hpp>

#include <string>

namespace lux {
namespace sys {
namespace graphic {

	using namespace unit_literals;

	void Terrain_comp::load(sf2::JsonDeserializer& state,
	                       asset::Asset_manager& assets){
		std::string material = _smart_texture.material() ? _smart_texture.material().aid().str() : "";
		std::vector<glm::vec2> points;
		points.reserve(16);
		auto shadowcaster = _smart_texture.shadowcaster();

		state.read_virtual(
			sf2::vmember("material", material),
			sf2::vmember("shadowcaster", shadowcaster),
			sf2::vmember("points", points)
		);

		if(!points.empty()) {
			_smart_texture.points(std::move(points));
		}
		_smart_texture.shadowcaster(shadowcaster);
		_smart_texture.material(assets.load<renderer::Material>(asset::AID(material)));
		INVARIANT(_smart_texture.material(), "Material '"<<material<<"' not found");
	}

	void Terrain_comp::save(sf2::JsonSerializer& state)const {
		std::string material = _smart_texture.material() ? _smart_texture.material().aid().str() : "";

		state.write_virtual(
			sf2::vmember("material", material),
			sf2::vmember("shadowcaster", _smart_texture.shadowcaster()),
			sf2::vmember("points", _smart_texture.points())
		);
	}

}
}
}
