#include "intro_screen.hpp"

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
		constexpr Time fade = 2_s;

		glm::vec2 offset = {0,0};
		float rotation = 0;
		float scale = 1;
	}

	Intro_screen::Intro_screen(Engine& game_engine) :
		Screen(game_engine),
	    _mailbox(game_engine.bus()),
	    _camera(calculate_vscreen(game_engine, 512)),
		_box(game_engine.assets(),  game_engine.assets().load<Texture>("tex:ui_intro"_aid),  _camera.viewport().w*2.f, _camera.viewport().w),
	    _box2(game_engine.assets(), game_engine.assets().load<Texture>("tex:ui_intro2"_aid), _camera.viewport().w*2.f, _camera.viewport().w),
	    _circle(game_engine.assets(), game_engine.assets().load<Texture>("tex:ui_intro_circle"_aid), _camera.viewport().w*2 *(800/2048.f), _camera.viewport().w*2 *(800/2048.f)),
	    _text_renderer(game_engine.assets()),
	    _debug_Text(game_engine.assets().load<Font>("font:menu_font"_aid)),
	    _fade_left(fade + 0.5_s),
	    _fadein_left(fade + fade/2 +1.0_s)
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

		_text_renderer.set_vp(_camera.vp());
	}

	void Intro_screen::_on_enter(util::maybe<Screen&> prev) {
		//_engine.audio_ctx().play_music(_engine.assets().load<audio::Music>("music:intro"_aid));
	}

	void Intro_screen::_on_leave(util::maybe<Screen&> next) {

	}

	void Intro_screen::_update(Time delta_time) {
		_mailbox.update_subscriptions();

//		_fade_left-=delta_time;
//		_fadein_left-=delta_time;

		auto p1 = _engine.input().pointer_screen_position(0).get_or_other({0,0});
		auto p2 = _engine.input().pointer_screen_position(1).get_or_other({0,0});
		_debug_Text.set("P1: "+util::to_string(p1.x)+" / "+util::to_string(p1.y)+"\n"+
						"P2: "+util::to_string(p2.x)+" / "+util::to_string(p2.y) );
	}


	void Intro_screen::_draw() {
		renderer::Disable_depthtest ddt{};
		(void)ddt;
/*
		auto fp = glm::clamp(_fade_left/fade, 0.f, 1.f);
		auto fg_color = 1-glm::clamp(_fadein_left/(fade/2), 0.f, 1.f);

		_box.set_vp(_camera.vp()
		            * glm::translate(glm::mat4(), {offset.x, offset.y, 0.f})
		             * glm::rotate(glm::mat4(), rotation, {0.f,0.f,1.f})
		             * glm::scale(glm::mat4(), {scale,scale,0.f}));
		_box.set_color({fp+0.18f,fp+0.18f,fp+0.18f,fp+0.18f});
		_box.draw(glm::vec2(0.f, 0.f));

		auto circle_color = (1-fp) - fg_color*0.85f;
		_circle.set_vp(_camera.vp() * glm::rotate(glm::mat4(), -_fade_left/20_s, {0.f,0.f,1.f}));
		_circle.set_color({circle_color,circle_color,circle_color,0.f});
		_circle.draw(glm::vec2(0.f, 0.f));

		if(fg_color>0) {
			_box2.set_vp(_camera.vp());
			_box2.set_color(glm::vec4{1,1,1,1}*fg_color);
			_box2.draw(glm::vec2(0.f, 0.f));
		}
*/

		_text_renderer.draw(_debug_Text, glm::vec2(0,0), glm::vec4(1,1,1,1), 0.5f);
	}
}
