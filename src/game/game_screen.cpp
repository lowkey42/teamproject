#define GLM_SWIZZLE

#include "game_screen.hpp"

#include "level.hpp"

#include <core/renderer/graphics_ctx.hpp>

#include <core/audio/music.hpp>
#include <core/audio/audio_ctx.hpp>

#include <core/input/events.hpp>
#include <core/input/input_manager.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace lux {
	using namespace unit_literals;
	using namespace renderer;

	Game_screen::Game_screen(Engine& engine, const std::string& level_id)
	    : Screen(engine),
	      _mailbox(engine.bus()),
	      _systems(engine),
	      _camera_ui(engine.graphics_ctx().viewport(),
	                   {engine.graphics_ctx().win_width(), engine.graphics_ctx().win_height()})
	{

		_mailbox.subscribe_to([&](input::Once_action& e){
			switch(e.id) {
				case "pause"_strid:
					_engine.screens().leave();
					break;
			}
		});
		_mailbox.subscribe_to([&](sys::physics::Collision& c){
			if(c.impact>40.f) {
				DEBUG("Smashed to death: "<<c.a<<" <=>"<<c.b<<" | "<<c.impact);
			}
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
		//_engine.audio_ctx().play_music(_engine.assets().load<audio::Music>("music:intro"_aid));
	}

	void Game_screen::_on_leave(util::maybe<Screen&> next) {
		_engine.input().screen_to_world_coords([](auto p) {
			return p;
		});
		_mailbox.disable();

	}

	void Game_screen::_update(Time dt) {
		_mailbox.update_subscriptions();

		_systems.update(dt, update_all);
	}


	void Game_screen::_draw() {
		_systems.draw();

		_render_queue.flush();
	}
}
