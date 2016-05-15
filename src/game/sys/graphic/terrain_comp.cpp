#include "terrain_comp.hpp"

#include <core/utils/sf2_glm.hpp>

#include <sf2/sf2.hpp>

#include <string>

namespace lux {
namespace sys {
namespace graphic {

	using namespace unit_literals;

	Terrain_comp::Terrain_comp(ecs::Entity& owner, renderer::Material_ptr material)
	    : Component(owner), _smart_texture(material, {{-1,-1}, {1,-1}, {1,1}, {-1,1}}) {
	}

	void Terrain_comp::load(sf2::JsonDeserializer& state,
	                       asset::Asset_manager& assets){
		std::string material = _smart_texture.material() ? _smart_texture.material().aid().str() : "";
		auto shadowcaster = _smart_texture.shadowcaster();
		auto decals_intensity = _smart_texture.decals_intensity();

		state.read_virtual(
			sf2::vmember("material", material),
			sf2::vmember("shadowcaster", shadowcaster),
			sf2::vmember("decals_intensity", decals_intensity)
		);

		_smart_texture.shadowcaster(shadowcaster);
		_smart_texture.decals_intensity(decals_intensity);
		_smart_texture.material(assets.load<renderer::Material>(asset::AID(material)));
		INVARIANT(_smart_texture.material(), "Material '"<<material<<"' not found");
	}

	void Terrain_comp::save(sf2::JsonSerializer& state)const {
		std::string material = _smart_texture.material() ? _smart_texture.material().aid().str() : "";

		state.write_virtual(
			sf2::vmember("material", material),
			sf2::vmember("shadowcaster", _smart_texture.shadowcaster()),
			sf2::vmember("decals_intensity", _smart_texture.decals_intensity())
		);
	}

	void Terrain_data_comp::load(sf2::JsonDeserializer& state,
	                       asset::Asset_manager&){
		std::vector<glm::vec2> points;
		points.reserve(16);

		state.read_virtual(
			sf2::vmember("points", points)
		);

		if(!points.empty()) {
			auto& terrain = owner().get<Terrain_comp>().get_or_throw();
			terrain.smart_texture().points(std::move(points));
		}
	}

	void Terrain_data_comp::save(sf2::JsonSerializer& state)const {
		auto& terrain = owner().get<Terrain_comp>().get_or_throw();

		state.write_virtual(
			sf2::vmember("points", terrain.smart_texture().points())
		);
	}

}
}
}
