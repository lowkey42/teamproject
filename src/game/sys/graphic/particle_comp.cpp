#define BUILD_SERIALIZER

#include "particle_comp.hpp"

#include <core/utils/sf2_glm.hpp>

#include <sf2/sf2.hpp>

#include <string>


namespace lux {
namespace sys {
namespace graphic {

	using namespace unit_literals;

	void Particle_comp::load(sf2::JsonDeserializer& state,
	                         asset::Asset_manager& assets) {
		auto types = std::vector<renderer::Particle_type_id>{};

		state.read_virtual(
			sf2::vmember("offset", _offset),
			sf2::vmember("emitters", types)
		);

		if(!types.empty()) {
			for(auto& e : _emitters) {
				e.reset();
			}
			for(auto& t : types)
				add(t);
		}
	}

	void Particle_comp::save(sf2::JsonSerializer& state)const {
		auto types = std::vector<renderer::Particle_type_id>{};
		types.reserve(_emitters.size());
		for(auto& e : _emitters) {
			if(e) {
				types.emplace_back(renderer::get_type(*e));
			}
		}

		state.write_virtual(
			sf2::vmember("offset", _offset),
			sf2::vmember("emitters", types)
		);
	}

	void Particle_comp::add(renderer::Particle_type_id id) {
		for(auto& q : _add_queue) {
			if(q==""_strid) {
				q = id;
				return;
			}
		}
	}
	void Particle_comp::remove(renderer::Particle_type_id id) {
		for(auto& e : _emitters) {
			if(e && id == renderer::get_type(*e)) {
				e.reset();
				return;
			}
		}
	}

}
}
}
