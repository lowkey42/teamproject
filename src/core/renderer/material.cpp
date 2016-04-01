#include "material.hpp"

#include "command_queue.hpp"


namespace lux {
namespace renderer {

	namespace {
		struct Material_desc {
			std::string albedo, normal, emission, roughness, metallic, height;
		};

		sf2_structDef(Material_desc, albedo, normal, emission,
		              roughness, metallic, height)

		Texture_ptr black;
	}

	Material::Material(asset::istream in) {
		Material_desc desc;

		sf2::deserialize_json(in, [&](auto& msg, uint32_t row, uint32_t column) {
			ERROR("Error parsing material from "<<in.aid().str()<<" at "<<row<<":"<<column<<": "<<msg);
		}, desc);

		auto load_or_default = [&](const auto& aid) {
			return aid.empty() ? black : in.manager().load<Texture>(asset::AID(aid));
		};

		_albedo    = load_or_default(desc.albedo);
		_normal    = load_or_default(desc.normal);
		_emission  = load_or_default(desc.emission);
		_roughness = load_or_default(desc.roughness);
		_metallic  = load_or_default(desc.metallic);
		_height    = load_or_default(desc.height);
	}

	void Material::set_textures(Command& cmd)const {
		cmd.texture(Texture_unit::color,     *_albedo);
		cmd.texture(Texture_unit::normal,    *_normal);
		cmd.texture(Texture_unit::emission,  *_emission);
		cmd.texture(Texture_unit::roughness, *_roughness);
		cmd.texture(Texture_unit::metallic,  *_metallic);
		cmd.texture(Texture_unit::height,    *_height);
	}

	void init_materials(asset::Asset_manager& assets) {
		black = assets.load<Texture>("tex:black"_aid);
	}
}
}
