#define GLM_SWIZZLE

#include "editor_screen.hpp"

#include "game_screen.hpp"

#include "sys/physics/transform_comp.hpp"

#include <core/units.hpp>
#include <core/renderer/graphics_ctx.hpp>
#include <core/renderer/command_queue.hpp>
#include <core/renderer/uniform_map.hpp>

#include <core/audio/music.hpp>
#include <core/audio/audio_ctx.hpp>

#include <core/input/events.hpp>
#include <core/input/input_manager.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace lux {
	using namespace unit_literals;
	using namespace renderer;

	namespace {
		struct Delete_cmd : util::Command {
			public:
				Delete_cmd(sys::editor::Selection& selection)
				    : _name("Entity deleted "+ecs::entity_name(selection.selection())),
				      _selection(selection) {}

				void execute()override {
					if(!_entity) {
						_entity = _selection.selection();
						INVARIANT(_entity, "No selected entity on execution of Delete_cmd");
						_saved_state = _entity->manager().backup(_entity);
					}

					_entity->manager().erase(_entity);
					_selection.select({});
				}
				void undo()override {
					INVARIANT(_entity, "No stored entity in Delete_cmd");
					_entity->manager().restore(_entity, _saved_state);
					_selection.select(_entity);
				}
				auto name()const -> const std::string& override{
					return _name;
				}

			private:
				const std::string _name;

				sys::editor::Selection& _selection;
				ecs::Entity_ptr _entity;
				std::string _saved_state;
		};

		struct Paste_cmd : util::Command {
			public:
				Paste_cmd(ecs::Entity_manager& ecs,
				          sys::editor::Selection& selection, std::string data, glm::vec3 pos)
				    : _name("Entity pasted"),
				      _ecs(ecs), _selection(selection), _data(data), _pos(pos) {}

				void execute()override {
					if(_entity) {
						_ecs.restore(_entity, _data);
					} else {
						_entity = _ecs.restore(_data);
						auto trans_comp = _entity->get<sys::physics::Transform_comp>();
						trans_comp.process([&](auto& t){
							t.position(_pos * 1_m);
						});
					}

					_selection.select(_entity);
				}
				void undo()override {
					INVARIANT(_entity, "No stored entity in Paste_cmd");
					_ecs.erase(_entity);
					_selection.select({});
				}
				auto name()const -> const std::string& override{
					return _name;
				}

			private:
				const std::string _name;

				ecs::Entity_manager& _ecs;
				sys::editor::Selection& _selection;
				ecs::Entity_ptr _entity;
				std::string _data;
				glm::vec3 _pos;
		};

	}


	Editor_screen::Editor_screen(Engine& engine, const std::string& level_id)
	    : Screen(engine),
	      _mailbox(engine.bus()),
	      _input_manager(engine.input()),
	      _systems(engine),
	      _camera_menu(engine.graphics_ctx().viewport(),
	                   {engine.graphics_ctx().win_width(), engine.graphics_ctx().win_height()}),
	      _camera_world(engine.graphics_ctx().viewport(), 80_deg, 5_m, 100_m),
	      _debug_Text(engine.assets().load<Font>("font:menu_font"_aid)),
	      _selection(engine, _systems.entity_manager, _camera_world, _commands),
	      _editor_sys(engine, _commands, _selection, _systems.entity_manager, engine.assets()),
	      _clipboard(util::nothing()),
	      _last_pointer_pos(util::nothing())
	{

		// TODO: move to method
		_mailbox.subscribe_to([&](input::Once_action& e){
			switch(e.id) {
				case "back"_strid:
					_engine.exit();
					break;

				case "undo"_strid:
					if(_commands.undo_available()) {
						DEBUG("Undo: "<<_commands.history().back());
						_commands.undo();
					}
					break;

				case "redo"_strid:
					if(_commands.redo_available()) {
						DEBUG("Redo: "<<_commands.future().front());
						_commands.redo();
					}
					break;

				case "toggle_grid"_strid:
					_selection.snap_to_grid(!_selection.snap_to_grid());
					DEBUG("Snap-To-Grid: "<<(_selection.snap_to_grid() ? "true" : "false"));
					break;

				case "zoom_in"_strid:
					if(_camera_world.position().z.value()  >= 2.5f)
						_camera_world.move(glm::vec3{0,0,-0.5} * 1_m);
					break;

				case "zoom_out"_strid:
					if(_camera_world.position().z.value()  <= 50.f)
						_camera_world.move(glm::vec3{0,0,0.5} * 1_m);
					break;

				case "delete"_strid:
					if(_selection.selection()) {
						_commands.execute<Delete_cmd>(_selection);
					}
					break;

				case "load"_strid:
					_level_metadata = _systems.load_level(_level_metadata.id);
					_selection.select({});
					_commands.clear();
					break;


				case "save"_strid:
					save_level(_engine, _systems.entity_manager, _level_metadata);
					break;

				case "start"_strid:
					save_level(_engine, _systems.entity_manager, _level_metadata);
					_engine.screens().enter<Game_screen>(_level_metadata.id);
					break;

				case "copy"_strid:
					if(_selection.selection()) {
						_clipboard = _selection.copy_content();
					}
					break;
				case "cut"_strid:
					if(_selection.selection()) {
						_clipboard = _selection.copy_content();
						_commands.execute<Delete_cmd>(_selection);
					}
					break;
				case "paste"_strid:
					_clipboard.process([&](auto& entity){
						auto pos = this->_camera_world.eye_position();
						pos.z = 0.f;
						_commands.execute<Paste_cmd>(_systems.entity_manager, _selection, entity, pos);
					});
					break;
			}
		});

		// TODO: move to method
		_mailbox.subscribe_to([&](input::Continuous_action& e) {
			switch(e.id) {
				case "scroll_l"_strid:
					_cam_speed.x += e.begin ? -1 : 1;
					break;
				case "scroll_r"_strid:
					_cam_speed.x += e.begin ? 1 : -1;
					break;
				case "scroll_d"_strid:
					_cam_speed.y += e.begin ? -1 : 1;
					break;
				case "scroll_u"_strid:
					_cam_speed.y += e.begin ? 1 : -1;
					break;
			}
		});

		_render_queue.shared_uniforms(renderer::make_uniform_map("vp", _camera_menu.vp()));

		_level_metadata = _systems.load_level(level_id);
	}

	void Editor_screen::_on_enter(util::maybe<Screen&> prev) {
		_engine.input().screen_to_world_coords([&](auto p) {
			return _camera_world.screen_to_world(p, glm::vec3(0,0,0)).xy();
		});
		_engine.input().enable_context("editor"_strid);
		_mailbox.enable();
	}

	void Editor_screen::_on_leave(util::maybe<Screen&> next) {
		_mailbox.disable();
		_engine.input().screen_to_world_coords([](auto p) {
			return p;
		});
	}

	auto Editor_screen::_handle_pointer_menu(util::maybe<glm::vec2> mp1,
	                                         util::maybe<glm::vec2> mp2) -> bool {
		if(mp1.is_nothing())
			return false;

		auto msrc = _camera_menu.screen_to_world(mp1.get_or_throw()).xy();

		auto blueprint_offset = glm::vec2 {
			_camera_menu.area().z, _camera_menu.area().x
		};
		auto blueprint = _editor_sys.find_blueprint(msrc, blueprint_offset);

		// TODO
		return blueprint.is_some();
	}
	auto Editor_screen::_handle_pointer_cam(util::maybe<glm::vec2> mp1,
	                                        util::maybe<glm::vec2>) -> bool {
		process(_last_pointer_pos,mp1) >> [&](auto last, auto curr) {
			auto wsrc = this->_camera_world.screen_to_world(last, glm::vec3(0,0,0));
			auto wtarget = this->_camera_world.screen_to_world(curr, glm::vec3(0,0,0));

			this->_camera_world.move((wsrc-wtarget)* 1_m);
			// TODO: multi-touch => zoom
		};
		return true;
	}

	void Editor_screen::_update(Time dt) {
		_mailbox.update_subscriptions();

		_systems.update(dt, Update::animations);
		_selection.update();
		_editor_sys.update(dt);

		if(_selection.selection()) {
			auto& transform = _selection.selection()->get<sys::physics::Transform_comp>().get_or_throw();
			auto pos = remove_units(transform.position());

			_debug_Text.set("Position: " + util::to_string(pos.x) + "/" +util::to_string(pos.y) +"/"+util::to_string(pos.z) );
		}

		auto mp1 = _input_manager.pointer_screen_position(0);
		auto mp2 = _input_manager.pointer_screen_position(1);

		bool unhandled = !_handle_pointer_menu(mp1,mp2)
		                 && !_selection.handle_pointer(mp1,mp2);

		if(unhandled) {
			_handle_pointer_cam(mp1,mp2);
		}

		_last_pointer_pos = mp1;

		_camera_world.move(glm::vec3(_cam_speed, 0.f) * dt.value() * 5_m);
	}


	void Editor_screen::_draw() {
		//_render_queue.shared_uniforms()->emplace("vp", _camera_world.vp());

		_systems.draw(_camera_world);
		_selection.draw(_render_queue, _camera_menu);

		auto blueprint_offset = glm::vec2 {
			_camera_menu.area().z, _camera_menu.area().x
		};

		_editor_sys.draw_blueprint_list(_render_queue, blueprint_offset);

		_debug_Text.draw(_render_queue, glm::vec2(-_camera_menu.size().x/2.f+_debug_Text.size().x/2.f*0.4f,
		                                          _camera_menu.size().y/2.f-_debug_Text.size().y/2.f*0.4f - 1.f), glm::vec4(1,1,1,1), 0.4f);

		_render_queue.flush();
	}
}
