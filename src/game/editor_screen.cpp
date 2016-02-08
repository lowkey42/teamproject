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
		glm::vec2 offset = {0,0};
		float rotation = 0;
		float scale = 1;
	}

	Editor_screen::Editor_screen(Engine& game_engine) :
		Screen(game_engine),
	    _mailbox(game_engine.bus()),
	    _systems(game_engine),
	    _camera_menu(calculate_vscreen(game_engine, 512)),
	    _camera_world(calculate_vscreen(game_engine, 512)),
	    _debug_Text(game_engine.assets().load<Font>("font:menu_font"_aid))
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
				case "move"_strid:
					offset+=e.value;
					break;

				case "rotate"_strid:
					rotation+=e.value.x;
					_mailbox.send<input::Force_feedback>(input::Input_source{0}, 1.0f);
					_mailbox.send<input::Force_feedback>(input::Input_source{1}, 1.0f);
					break;

				case "zoom"_strid:
					scale=glm::clamp(scale * (e.value.x/50), 0.01f, 10.f);
					_mailbox.send<input::Force_feedback>(input::Input_source{0}, 1.0f);
					_mailbox.send<input::Force_feedback>(input::Input_source{1}, 1.0f);
					break;
			}
		});

		_render_queue.shared_uniforms(renderer::make_uniform_map("VP", _camera_menu.vp()));

		auto& transform = _systems.entity_manager.emplace("blueprint:test"_aid)->get<sys::physics::Transform_comp>().get_or_throw();
		transform.position(Position{1_m, 1_m});
	}

	void Editor_screen::_on_enter(util::maybe<Screen&> prev) {
		//_engine.audio_ctx().play_music(_engine.assets().load<audio::Music>("music:intro"_aid));
	}

	void Editor_screen::_on_leave(util::maybe<Screen&> next) {

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

		_debug_Text.draw(_render_queue,  glm::vec2(0,0), glm::vec4(1,1,1,1), 0.5f);

		_render_queue.flush();
	}
}
