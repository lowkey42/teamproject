#include "editor_screen.hpp"

#include "../core/units.hpp"
#include "../core/renderer/graphics_ctx.hpp"

#include <core/audio/music.hpp>
#include <core/audio/audio_ctx.hpp>

#include <core/input/events.hpp>
#include <core/input/input_manager.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace mo {
	using namespace unit_literals;
	using namespace renderer;

	namespace {
		auto is_inside(const ecs::Entity& entity, glm::vec2 p) {
			return false; // TODO
		}
	}

	Editor_screen::Editor_screen(Engine& engine) :
		Screen(engine),
	    _mailbox(engine.bus()),
	    _systems(engine),
	    _editor_sys(engine.assets()),
	    _camera_menu({engine.graphics_ctx().win_width(), engine.graphics_ctx().win_height()}),
	    _camera_world(calculate_vscreen(engine, 512)),
	    _debug_Text(engine.assets().load<Font>("font:menu_font"_aid))
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

		_mailbox.subscribe_to([&](input::Range_action& e){
			switch(e.id) {
				case "drag"_strid:
					_on_drag(e.abs - e.rel, e.abs);
					break;

				case "mouse_down"_strid:
					break;
			}
		});

		_render_queue.shared_uniforms(renderer::make_uniform_map("VP", _camera_menu.vp()));

		auto& transform = _systems.entity_manager.emplace("blueprint:test"_aid)->get<sys::physics::Transform_comp>().get_or_throw();
		transform.position(Position{0_m, 0_m});
	}

	void Editor_screen::_on_enter(util::maybe<Screen&> prev) {
		//_engine.audio_ctx().play_music(_engine.assets().load<audio::Music>("music:intro"_aid));
	}

	void Editor_screen::_on_leave(util::maybe<Screen&> next) {

	}

	void Editor_screen::_on_drag(glm::vec2 src, glm::vec2 target) {
		auto wsrc = _camera_world.screen_to_world(src);
		auto wtarget = _camera_world.screen_to_world(target);

		auto blueprint_offset = glm::vec2 {
			_camera_menu.area().z, _camera_menu.area().x
		};
		auto blueprint = _editor_sys.find_blueprint(src, blueprint_offset, _camera_menu);

		if(blueprint.is_some()) {
			DEBUG("Blueprint drag");

		} else if(_selected_entity && is_inside(*_selected_entity, wsrc)) {
			// TODO
		} else {
			_camera_world.move((wsrc-wtarget) /2.f);
		}
	}

	void Editor_screen::_update(Time dt) {
		_mailbox.update_subscriptions();

		_systems.update(dt, Update::movements);

		auto p1 = _engine.input().pointer_screen_position(0).get_or_other({0,0});
		auto p2 = _engine.input().pointer_screen_position(1).get_or_other({0,0});
		_debug_Text.set("P1: "+util::to_string(p1.x)+" / "+util::to_string(p1.y)+"\n"+
						"P2: "+util::to_string(p2.x)+" / "+util::to_string(p2.y) );
	}


	void Editor_screen::_draw() {
		_systems.draw(_camera_world);

		auto blueprint_offset = glm::vec2 {
			_camera_menu.area().z, _camera_menu.area().x
		};

		_editor_sys.draw_blueprint_list(_render_queue, blueprint_offset);

		_debug_Text.draw(_render_queue,  glm::vec2(0,0), glm::vec4(1,1,1,1), 0.1f);

		_render_queue.flush();
	}
}
