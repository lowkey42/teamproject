#include "editor_system.hpp"


namespace mo {
namespace sys {
namespace editor {

	using namespace renderer;
	using namespace glm;
	using namespace unit_literals;


	struct Editor_conf {
		int columns;
		float icon_size;
		std::string default_category;
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
	sf2_structDef(Editor_conf, columns, icon_size, default_category, blueprints)

	Editor_system::Editor_system(asset::Asset_manager& assets)
	    : _conf(assets.load<Editor_conf>("cfg:editor"_aid)) {

		_current_category = _conf->default_category;
	}

	void Editor_system::draw_blueprint_list(renderer::Command_queue& queue,
	                                        glm::vec2 offset) {

		auto& blueprints = util::find_maybe(_conf->blueprints, _current_category).get_or_throw();

		const auto base_pos = vec3{
			offset.x - _conf->icon_size*_conf->columns,
			offset.y, 0.f
		};
		auto column = 0.f;
		auto row = 0.f;

		for(auto& blueprint : blueprints) {
			auto w = blueprint.icon->width();
			auto h = blueprint.icon->width();
			if(w>=h) {
				h *= _conf->icon_size / w;
				w = _conf->icon_size;
			} else {
				w *= _conf->icon_size / h;
				h = _conf->icon_size;
			}

			const auto pos = base_pos + vec3 {
				(column+0.5f) * _conf->icon_size,
				(row+0.5f) * _conf->icon_size,
				0.f
			};

			_icon_batch.insert(Sprite{pos, 0_deg, vec2{w,h}, glm::vec4{0,0,1,1}, *blueprint.icon});

			column++;
			if(column >= _conf->columns) {
				column = 0;
				row++;
			}
		}

		_icon_batch.flush(queue);
	}

	auto Editor_system::find_blueprint(
	        glm::vec2 screen_position,
	        const renderer::Camera& camera
	        )const -> util::maybe<const Entity_blueprint_info&> {

		auto pos = camera.screen_to_world(screen_position);

		pos = glm::floor(pos / _conf->icon_size);

		auto index = static_cast<std::size_t>(pos.x + pos.y * _conf->columns);

		auto cat_iter = _conf->blueprints.find(_current_category);
		if(cat_iter==_conf->blueprints.end())
			return util::nothing();

		if(index>=cat_iter->second.size())
			return util::nothing();

		return util::justPtr(&cat_iter->second[index]);
	}

}
}
}
