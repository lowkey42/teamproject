#include "editor_system.hpp"


namespace lux {
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

	Editor_system::Editor_system(ecs::Entity_manager& entity_manager, asset::Asset_manager& assets)
	    : _conf(assets.load<Editor_conf>("cfg:editor"_aid)) {

		entity_manager.register_component_type<Editor_comp>();

		_current_category = _conf->default_category;
	}

	void Editor_system::draw_blueprint_list(renderer::Command_queue& queue,
	                                        glm::vec2 offset) {

		auto& blueprints = util::find_maybe(_conf->blueprints, _current_category).get_or_throw();

		const auto base_pos = vec2{
			offset.x - _conf->icon_size*_conf->columns,
			offset.y + _conf->icon_size
		};
		auto column = 0.f;
		auto row = 1.f;

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

			const auto pos = base_pos + vec2 {
				(column+0.5f) * _conf->icon_size,
				(row+0.5f) * _conf->icon_size
			};

			_icon_batch.insert(*blueprint.icon, pos, vec2{w,h});

			column++;
			if(column >= _conf->columns) {
				column = 0;
				row++;
			}
		}

		_icon_batch.flush(queue);
	}

	auto Editor_system::find_blueprint(
	        glm::vec2 click_position,
	        glm::vec2 offset
	        )const -> util::maybe<const Entity_blueprint_info&> {

		const auto base_pos = vec2 {
			offset.x - _conf->icon_size*_conf->columns,
			offset.y + _conf->icon_size*2
		};
		auto pos = click_position - base_pos;

		pos = glm::floor(pos / _conf->icon_size);

		if(pos.x<0 || pos.x>=_conf->columns || pos.y<0)
			return util::nothing();

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
