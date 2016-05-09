#include "editor_system.hpp"

#include "selection.hpp"

#include "../physics/transform_comp.hpp"

#include <core/input/input_manager.hpp>


namespace lux {
namespace sys {
namespace editor {

	using namespace renderer;
	using namespace glm;
	using namespace unit_literals;

	namespace {
		constexpr auto icon_size = 64.f;

		struct Create_cmd : util::Command {
			public:
				Create_cmd(ecs::Entity_manager& ecs,
						  sys::editor::Selection& selection, std::string blueprint,
						   glm::vec3 pos)
					: _name("Entity created"),
					  _ecs(ecs), _selection(selection), _blueprint(blueprint), _pos(pos) {}

				void execute()override {
					if(_entity) {
						_ecs.restore(_entity, _data);
					} else {
						_entity = _ecs.emplace(asset::AID{"blueprint"_strid, _blueprint});
						auto trans_comp = _entity->get<sys::physics::Transform_comp>();
						trans_comp.process([&](auto& t){
							t.position(_pos * 1_m);
						});
					}

					_entity_prev_selected = _selection.selection();
					_selection.select(_entity);
				}
				void undo()override {
					INVARIANT(_entity, "No stored entity in Create_cmd");
					_data = _ecs.backup(_entity);
					_ecs.erase(_entity);
					_selection.select(_entity_prev_selected);
				}
				auto name()const -> const std::string& override{
					return _name;
				}

			private:
				const std::string _name;

				ecs::Entity_manager& _ecs;
				sys::editor::Selection& _selection;
				ecs::Entity_ptr _entity;
				ecs::Entity_ptr _entity_prev_selected;
				std::string _blueprint;
				std::string _data;
				glm::vec3 _pos;
		};
	}

	struct Blueprint_group {
		std::string icon;
		std::vector<Entity_blueprint_info> blueprints;
	};

	struct Editor_conf {
		int columns;
		std::string default_category;
		std::unordered_map<std::string, Blueprint_group> blueprint_groups;
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
	sf2_structDef(Blueprint_group, icon, blueprints)
	sf2_structDef(Editor_conf, columns, default_category, blueprint_groups)

	Editor_system::Editor_system(Engine& e, util::Command_manager& commands, Selection& selection,
	                             ecs::Entity_manager& entity_manager, asset::Asset_manager& assets)
	    : _engine(e), _mailbox(e.bus()), _commands(commands), _selection(selection),
	      _entity_manager(entity_manager),
	      _conf(assets.load<Editor_conf>("cfg:editor"_aid)) {

		entity_manager.register_component_type<Editor_comp>();

		_current_category = _conf->default_category;

		_mailbox.subscribe_to([&](input::Once_action& e) {
			switch(e.id) {
				case "new_a"_strid:
					_spawn_new(0, _engine.input().last_pointer_world_position());
					break;
				case "new_b"_strid:
					_spawn_new(1, _engine.input().last_pointer_world_position());
					break;
				case "new_c"_strid:
					_spawn_new(2, _engine.input().last_pointer_world_position());
					break;
				case "new_d"_strid:
					_spawn_new(3, _engine.input().last_pointer_world_position());
					break;
				case "new_e"_strid:
					_spawn_new(4, _engine.input().last_pointer_world_position());
					break;
				case "new_f"_strid:
					_spawn_new(5, _engine.input().last_pointer_world_position());
					break;
				case "new_g"_strid:
					_spawn_new(6, _engine.input().last_pointer_world_position());
					break;
				case "new_h"_strid:
					_spawn_new(7, _engine.input().last_pointer_world_position());
					break;
				case "new_i"_strid:
					_spawn_new(8, _engine.input().last_pointer_world_position());
					break;
			}
		});
	}

	void Editor_system::_spawn_new(std::size_t index, glm::vec2 pos) {
		// get blueprint by index
		auto cat_iter = _conf->blueprint_groups.find(_current_category);
		if(cat_iter==_conf->blueprint_groups.end())
			return;

		if(index>=cat_iter->second.blueprints.size())
			return;

		auto blueprint = cat_iter->second.blueprints.at(index).id;

		_commands.execute<Create_cmd>(_entity_manager, _selection, blueprint, glm::vec3{pos, 0.f});
	}

	void Editor_system::update(Time) {
		_mailbox.update_subscriptions();
	}

	void Editor_system::draw_blueprint_list(renderer::Command_queue& queue,
	                                        glm::vec2 offset) {

		auto& blueprints = util::find_maybe(_conf->blueprint_groups, _current_category).get_or_throw().blueprints;

		const auto base_pos = vec2{
			offset.x - icon_size*_conf->columns,
			offset.y + icon_size
		};
		auto column = 0.f;
		auto row = 1.f;

		for(auto& blueprint : blueprints) {
			auto w = blueprint.icon->width();
			auto h = blueprint.icon->width();
			if(w>=h) {
				h *= icon_size / w;
				w = icon_size;
			} else {
				w *= icon_size / h;
				h = icon_size;
			}

			const auto pos = base_pos + vec2 {
				(column+0.5f) * icon_size,
				(row+0.5f) * icon_size
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
			offset.x - icon_size*_conf->columns,
			offset.y + icon_size*2
		};
		auto pos = click_position - base_pos;

		pos = glm::floor(pos / icon_size);

		if(pos.x<0 || pos.x>=_conf->columns || pos.y<0)
			return util::nothing();

		auto index = static_cast<std::size_t>(pos.x + pos.y * _conf->columns);

		auto cat_iter = _conf->blueprint_groups.find(_current_category);
		if(cat_iter==_conf->blueprint_groups.end())
			return util::nothing();

		if(index>=cat_iter->second.blueprints.size())
			return util::nothing();

		return util::justPtr(&cat_iter->second.blueprints[index]);
	}

}
}
}
