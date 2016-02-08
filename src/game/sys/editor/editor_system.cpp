#include "editor_system.hpp"


namespace mo {
namespace sys {
namespace editor {

	struct Editor_conf {
		int columns;
		float icon_size;
		std::unordered_map<std::string, std::vector<Entity_blueprint_info>> blueprints;
	};

	void load(sf2::JsonDeserializer ds, Entity_blueprint_info& t) {
		std::string icon_aid = t.icon ? t.icon.aid().str() : "";

		ds.read_virtual(
			sf2::vmember("id", t.id),
			sf2::vmember("icon", icon_aid)
		);

		t.icon = asset::get_asset_manager().load<renderer::Texture>(asset::AID(icon_aid));
	}
	void save(sf2::JsonSerializer ds, const Entity_blueprint_info& t) {
		std::string icon_aid = t.icon ? t.icon.aid().str() : "";

		ds.write_virtual(
			sf2::vmember("id", t.id),
			sf2::vmember("icon", icon_aid)
		);
	}

	sf2_structDef(Entity_blueprint_info, id, icon)
	sf2_structDef(Editor_conf, blueprints)

	Editor_system::Editor_system(asset::Asset_manager& assets)
	    : _conf(assets.load<Editor_conf>("conf:editor"_aid)) {
	}

	void Editor_system::draw_blueprint_list(const renderer::Camera& camera) {
		// TODO
	}

	auto Editor_system::find_blueprint(
	        glm::vec2 screen_position,
	        const renderer::Camera& camera
	        )const -> util::maybe<const Entity_blueprint_info&> {

		auto pos = camera.screen_to_world(screen_position);

		pos = glm::floor(pos / _conf->icon_size);

		auto index = static_cast<std::size_t>(pos.x + pos.y * _conf->columns);

		auto cat_iter = _conf->blueprints.find(current_category);
		if(cat_iter==_conf->blueprints.end())
			return util::nothing();

		if(index>=cat_iter->second.size())
			return util::nothing();

		return util::justPtr(&cat_iter->second[index]);
	}

}
}
}
