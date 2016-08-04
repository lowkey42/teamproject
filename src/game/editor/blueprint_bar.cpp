#define GLM_SWIZZLE

#include "blueprint_bar.hpp"

#include "selection.hpp"
#include "editor_cmds.hpp"

#include "../sys/physics/transform_comp.hpp"

#include <core/input/input_manager.hpp>


namespace lux {
namespace editor {

	using namespace renderer;
	using namespace glm;
	using namespace unit_literals;

	namespace {
		constexpr auto icon_size = 92.f;
		constexpr auto max_columns = 2;

		bool is_inside(glm::vec2 p, glm::vec2 center, glm::vec2 size) {
			return p.x>=center.x-size.x/2.f && p.x<=center.x+size.x/2.f &&
			       p.y>=center.y-size.y/2.f && p.y<=center.y+size.y/2.f;
		}
	}

	struct Blueprint_desc {
		std::string id;
		std::string icon;
		renderer::Texture_ptr icon_texture;
	};
	struct Blueprint_group {
		std::string name;
		std::string icon;
		renderer::Texture_ptr icon_texture;
		std::vector<Blueprint_desc> blueprints;
	};

	struct Editor_conf {
		std::vector<Blueprint_group> blueprint_groups;
	};

	sf2_structDef(Blueprint_desc, id, icon)
	sf2_structDef(Blueprint_group, name, icon, blueprints)
	sf2_structDef(Editor_conf, blueprint_groups)

}

namespace asset {
	template<>
	struct Loader<editor::Editor_conf> {
		static auto load(istream in) {
			auto r = std::make_shared<editor::Editor_conf>();

			sf2::deserialize_json(in, [&](auto& msg, uint32_t row, uint32_t column) {
				ERROR("Error parsing JSON from "<<in.aid().str()<<" at "<<row<<":"<<column<<": "<<msg);
			}, *r);

			for(auto& group : r->blueprint_groups) {
				group.icon_texture = in.manager().load<renderer::Texture>(asset::AID{group.icon});
				for(auto& blueprint : group.blueprints) {
					blueprint.icon_texture = in.manager().load<renderer::Texture>(asset::AID{blueprint.icon});
				}
			}

			return r;
		}

		static void store(ostream, const editor::Editor_conf&) {
			FAIL("NOT IMPLEMENTED!");
		}
	};
}

namespace editor {

	Blueprint_bar::Blueprint_bar(Engine& e, util::Command_manager& commands, Selection& selection,
	                             ecs::Entity_manager& entity_manager, asset::Asset_manager& assets,
	                             input::Input_manager& input_manager,
	                             renderer::Camera& camera_world, renderer::Camera& camera_ui,
				                 glm::vec2 offset)
	    : _engine(e),
	      _camera_world(camera_world), _camera_ui(camera_ui), _offset(offset),
	      _mailbox(e.bus()), _commands(commands), _selection(selection),
	      _entity_manager(entity_manager), _input_manager(input_manager),
	      _background(e.assets().load<renderer::Texture>("tex:editor_bar_blueprints"_aid)),
	      _back_button(e.assets().load<renderer::Texture>("tex:editor_bar_blueprints_back"_aid)),
	      _conf(assets.load<Editor_conf>("cfg:editor"_aid)) {

		entity_manager.register_component_type<Editor_comp>();

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

		_mailbox.subscribe_to([&](input::File_dropped& e){
			_engine.assets().find_by_path(e.path).process([&](auto aid) {
				if(aid.type()=="blueprint"_strid) {
					auto pos = _engine.input().last_pointer_world_position();
					_commands.execute<Create_cmd>(_entity_manager, _selection, aid.name(), glm::vec3{pos, 0.f});
				}
			});
		});
	}

	void Blueprint_bar::_spawn_new(std::size_t index, glm::vec2 pos) {
		if(_current_category.is_some()) {
			auto& blueprints = _current_category.get_or_throw().blueprints;
			if(index < blueprints.size()) {
				auto& id = blueprints.at(index).id;
				_commands.execute<Create_cmd>(_entity_manager, _selection, id, glm::vec3{pos, 0.f});
			}

		} else if(index < _conf->blueprint_groups.size()) {
			_current_category = _conf->blueprint_groups.at(index);
		}
	}

	void Blueprint_bar::draw(renderer::Command_queue& queue) {
		auto offset = _offset - glm::vec2{_background->width()/2.f, 0.f};
		auto size = glm::vec2{_background->width(), _background->height()};
		_batch.insert(*_background, offset, size);

		if(_current_category.is_some()) {
			offset += glm::vec2{0, -_background->height()/2.f + 8 + _back_button->height()/6.f};
			size = glm::vec2{_back_button->width(), _back_button->height()/3};
			auto button_clip = 0.f;
			if(is_inside(_last_mouse_pos, offset, size)) {
				button_clip = _mouse_pressed ? 2.f/3 : 1.f/3;
			}
			_batch.insert(*_back_button, offset, size, 0_deg, {0,button_clip,1,button_clip+1.f/3});
		}

		if(_dragging.is_some() && _current_category.is_some()) {
			auto index = _dragging.get_or_throw();

			auto& blueprints = _current_category.get_or_throw().blueprints;
			if(index < blueprints.size()) {
				_batch.insert(*blueprints.at(index).icon_texture, _last_mouse_pos, glm::vec2{icon_size,icon_size});
			}
		}

		auto draw_icons = [&](auto& list) {
			auto offset = _offset - glm::vec2{_background->width(), _background->height()/2.f}
			              + glm::vec2{8.f + icon_size/2.f, 58.f + icon_size/2.f};
			auto margin = 4.f + icon_size;
			auto column = 0;
			auto row = 0;
			for(auto& e : list) {
				auto icon_offset = offset + glm::vec2{column, row}*margin;
				_batch.insert(*e.icon_texture, icon_offset, glm::vec2{icon_size,icon_size});
				column++;
				if(column>=max_columns) {
					column=0;
					row++;
				}
			}
		};

		if(_current_category.is_some()) {
			draw_icons(_current_category.get_or_throw().blueprints);
		} else {
			draw_icons(_conf->blueprint_groups);
		}

		_batch.flush(queue);
	}
	void Blueprint_bar::update() {
		_mailbox.update_subscriptions();
	}
	auto Blueprint_bar::handle_pointer(util::maybe<glm::vec2> mp1, util::maybe<glm::vec2>) -> bool {
		auto offset = _offset - glm::vec2{_background->width()/2.f, 0.f};
		auto size = glm::vec2{_background->width(), _background->height()};

		auto mouse_released = _mouse_pressed && !mp1.is_some();
		_mouse_pressed = mp1.is_some();

		auto mouse_pos = mp1.get_or_other(_input_manager.last_pointer_screen_position());
		_last_mouse_pos = _camera_ui.screen_to_world(mouse_pos, 1.f).xy();

		if(_dragging.is_some()) {
			if(mouse_released) {
				if(!is_inside(_last_mouse_pos, offset, size) || _current_category.is_nothing()) {
					_spawn_new(_dragging.get_or_throw(), _input_manager.last_pointer_world_position());
				}
				_dragging = util::nothing();
			}

			return true;
		}

		if(is_inside(_last_mouse_pos, offset, size)) {
			offset += glm::vec2{0, -_background->height()/2.f + 8 + _back_button->height()/6.f};
			size = glm::vec2{_back_button->width(), _back_button->height()/3};

			if(is_inside(_last_mouse_pos, offset, size)) {
				if(mouse_released)
					_current_category = util::nothing();

			} else if(_mouse_pressed) {
				auto offset = _offset - glm::vec2{_background->width(), _background->height()/2.f}
				              + glm::vec2{8.f + icon_size/2.f, 58.f + icon_size/2.f};
				auto margin = 4.f + icon_size;

				auto p = _last_mouse_pos - offset;
				p /= margin;

				auto index = std::round(p.x) + std::round(p.y)*max_columns;

				if(index>=0) {
					_dragging = index;
				}
			}
			return true;
		}
		return false;
	}

	auto Blueprint_bar::is_in_delete_zone(util::maybe<glm::vec2> mp1) -> bool {
		if(mp1.is_nothing())
			return false;

		auto mouse_pos = mp1.get_or_other(mp1.get_or_throw());
		auto p = _camera_ui.screen_to_world(mouse_pos, 1.f).xy();

		return is_inside(p, _offset+glm::vec2{-_background->width()/2.f, _background->height()/2.f-52}, glm::vec2{188,88});
	}

}
}
