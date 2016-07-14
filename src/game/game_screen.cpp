#define GLM_SWIZZLE

#include "game_screen.hpp"
#include "level.hpp"
#include "loading_screen.hpp"

#include <core/renderer/graphics_ctx.hpp>

#include <core/audio/music.hpp>
#include <core/audio/sound.hpp>
#include <core/audio/audio_ctx.hpp>

#include <core/input/events.hpp>
#include <core/input/input_manager.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace lux {
	using namespace unit_literals;
	using namespace renderer;

	namespace {
		constexpr auto fadeout_delay = 2_s;
		const auto fadeout_sun = Rgb{1.8, 1.75, 0.78} *4.f;
	}

	Game_screen::Game_screen(Engine& engine, const std::string& level_id)
		: Screen(engine),
		  _mailbox(engine.bus()),
		  _systems(engine),
		  _ui_text(engine.assets().load<Font>("font:menu_font"_aid)),
		  _camera_ui(engine.graphics_ctx().viewport(),
					   {engine.graphics_ctx().win_width(), engine.graphics_ctx().win_height()}),
		  _current_level(level_id)
	{

		_mailbox.subscribe_to([&](input::Once_action& e){
			switch(e.id) {
				case "pause"_strid:
					_engine.screens().leave();
					break;
			}
		});

		_mailbox.subscribe_to([&](sys::gameplay::Level_finished& e){
			DEBUG("reached the end of the level");
			_fadeout = true;
			_engine.audio_ctx().stop_music(fadeout_delay*0.75f);
		});

		_render_queue.shared_uniforms(renderer::make_uniform_map("vp", _camera_ui.vp()));

		_systems.load_level(level_id);
	}

	void Game_screen::_on_enter(util::maybe<Screen&> prev) {
		_engine.input().enable_context("game"_strid);
		_mailbox.enable();
		_engine.input().screen_to_world_coords([&](auto p) {
			return _systems.camera.screen_to_world(p).xy();
		});
		_engine.audio_ctx().play_music(_engine.assets().load<audio::Music>("music:game.ogg"_aid), 1_s);
	}

	void Game_screen::_on_leave(util::maybe<Screen&> next) {
		_engine.input().screen_to_world_coords([](auto p) {
			return p;
		});
		_mailbox.disable();
		_engine.audio_ctx().stop_music();

	}

	void Game_screen::_update(Time dt) {
		_mailbox.update_subscriptions();

		if(_fadeout)
			_systems.update(dt, Update::animations | Update::movements | Update::gameplay);
		else
			_systems.update(dt, update_all);

		if(_systems.gameplay.game_time()>0.0_s)
			_ui_text.set(util::to_string(_systems.gameplay.game_time().value())+"s");
		else
			_ui_text.set("");

		if(_fadeout) {
			_fadeout_fadetimer+=dt;

			_systems.light_config(glm::mix(_systems.lights.sun_light(), fadeout_sun, _fadeout_fadetimer/fadeout_delay),
								  _systems.lights.sun_dir(),
								  _systems.lights.ambient_brightness(),
								  _systems.lights.background_tint() );

			if(_fadeout_fadetimer>=fadeout_delay) {
				unlock_next_levels(_engine, _current_level);
				_engine.screens().leave();
			}
		}
	}


	void Game_screen::_draw() {
		_systems.draw();

		_ui_text.draw(_render_queue, glm::vec2(-_camera_ui.size().x/2.f+_ui_text.size().x/2.f*0.5f + 1.f,
											   -_camera_ui.size().y/2.f+_ui_text.size().y/2.f*0.5f + 1.f), glm::vec4(1,1,1,1), 0.5f);

		_render_queue.flush();
	}
}
