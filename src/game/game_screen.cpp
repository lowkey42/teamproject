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
				case "back"_strid:
					_engine.screens().leave();
					break;
			}
		});

		_render_queue.shared_uniforms(renderer::make_uniform_map("vp", _camera_ui.vp()));

		_systems.load_level(level_id);
	}

	void Game_screen::_on_enter(util::maybe<Screen&> prev) {
		_mailbox.enable();
		//_engine.audio_ctx().play_music(_engine.assets().load<audio::Music>("music:intro"_aid));
	}

	void Game_screen::_on_leave(util::maybe<Screen&> next) {
		_mailbox.disable();

	}

	void Game_screen::_update(Time dt) {
		_mailbox.update_subscriptions();

		_systems.update(dt, Update::movements);
	}


	void Game_screen::_draw() {
		_systems.draw();

		_render_queue.flush();
	}
}
