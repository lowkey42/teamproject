#define GLM_SWIZZLE

#include "editor_screen.hpp"

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


namespace mo {
	using namespace unit_literals;
	using namespace renderer;


	Editor_screen::Editor_screen(Engine& engine)
	    : Screen(engine),
	      _mailbox(engine.bus()),
	      _input_manager(engine.input()),
	      _systems(engine),
	      _editor_sys(_systems.entity_manager, engine.assets()),
	      _camera_menu(engine.graphics_ctx().viewport(),
	                   {engine.graphics_ctx().win_width(), engine.graphics_ctx().win_height()}),
	      _camera_world(engine.graphics_ctx().viewport(), 80_deg, -5_m, 20_m),
	      _debug_Text(engine.assets().load<Font>("font:menu_font"_aid)),
	      _selection(engine, _systems.entity_manager, _camera_world, _commands),
	      _last_pointer_pos(util::nothing())
	{

		_mailbox.subscribe_to([&](input::Once_action& e){
			switch(e.id) {
				case "back"_strid:
					_engine.exit();
					break;

				case "test_ff_a"_strid:
					_mailbox.send<input::Force_feedback>(input::Input_source{1}, 1.f);
					break;
				case "test_ff_b"_strid:
					_mailbox.send<input::Force_feedback>(input::Input_source{1}, 0.2f);
					break;
			}
		});


		_render_queue.shared_uniforms(renderer::make_uniform_map("vp", _camera_menu.vp()));

		{
			_selected_entity = _systems.entity_manager.emplace("blueprint:test"_aid);
			auto& transform = _selected_entity->get<sys::physics::Transform_comp>().get_or_throw();
			transform.position(Position{0_m, -3_m, 0_m});
		}
		{
			auto& transform = _systems.entity_manager.emplace("blueprint:test_light"_aid)->get<sys::physics::Transform_comp>().get_or_throw();
			transform.position(Position{0_m, 0_m, 2.0_m});
		}
		{
			auto& transform = _systems.entity_manager.emplace("blueprint:test_bg"_aid)->get<sys::physics::Transform_comp>().get_or_throw();
			transform.position(Position{0_m, 0_m, -2.0_m});
		}
		_selection.select(_selected_entity);
	}

	void Editor_screen::_on_enter(util::maybe<Screen&> prev) {
		//_engine.audio_ctx().play_music(_engine.assets().load<audio::Music>("music:intro"_aid));
	}

	void Editor_screen::_on_leave(util::maybe<Screen&> next) {

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
			INFO("cam movement");
			auto wsrc = this->_camera_world.screen_to_world(last, glm::vec3(0,0,0));
			auto wtarget = this->_camera_world.screen_to_world(curr, glm::vec3(0,0,0));

			this->_camera_world.move((wsrc-wtarget)* 1_m);
		};
		return true;
	}

	void Editor_screen::_update(Time dt) {
		_mailbox.update_subscriptions();

		_systems.update(dt, Update::movements);
		_selection.update();

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
	}


	void Editor_screen::_draw() {
		//_render_queue.shared_uniforms()->emplace("vp", _camera_world.vp());

		_systems.draw(_camera_world);
		_selection.draw(_render_queue, _camera_menu);

		auto blueprint_offset = glm::vec2 {
			_camera_menu.area().z, _camera_menu.area().x
		};

		_editor_sys.draw_blueprint_list(_render_queue, blueprint_offset);

		_debug_Text.draw(_render_queue,  glm::vec2(0,0), glm::vec4(1,1,1,1), 1.0f);

		_render_queue.flush();
	}
}
