#define BUILD_SERIALIZER

#include "decal_comp.hpp"

#include <core/utils/sf2_glm.hpp>
#include <core/ecs/serializer.hpp>

#include <sf2/sf2.hpp>

#include <string>

namespace lux {
namespace sys {
namespace graphic {

	using namespace unit_literals;

	void load_component(ecs::Deserializer& state, Decal_comp& comp) {
		std::string aid = comp._texture ? comp._texture.aid().str() : "";

		state.read_virtual(
			sf2::vmember("texture", aid),
			sf2::vmember("size", comp._size)
		);

		if(!aid.empty())
			comp._texture = comp.manager().userdata().assets().load<renderer::Texture>(asset::AID(aid));
	}
	void save_component(ecs::Serializer& state, const Decal_comp& comp) {
		std::string aid = comp._texture ? comp._texture.aid().str() : "";

		state.write_virtual(
			sf2::vmember("texture", aid),
			sf2::vmember("size", comp._size)
		);
	}

}
}
}
