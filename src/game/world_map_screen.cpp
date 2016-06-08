#define GLM_SWIZZLE

#include "world_map_screen.hpp"
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


	World_map_screen::World_map_screen(Engine& engine, const std::string& level_pack_id)
	    : Screen(engine),
	      _mailbox(engine.bus()),
	      _ui_text(engine.assets().load<Font>("font:menu_font"_aid)),
	      _camera_ui(engine.graphics_ctx().viewport(),
	                   {engine.graphics_ctx().win_width(), engine.graphics_ctx().win_height()}),
	      _level_pack(get_level_pack(engine,level_pack_id))
	{

		_mailbox.subscribe_to([&](input::Once_action& e){
			switch(e.id) {
				case "pause"_strid:
					_engine.screens().leave();
					break;
			}
		});

		_render_queue.shared_uniforms(renderer::make_uniform_map("vp", _camera_ui.vp()));
	}

	void World_map_screen::_on_enter(util::maybe<Screen&> prev) {
		_engine.input().enable_context("menu"_strid);
		_mailbox.enable();
	}

	void World_map_screen::_on_leave(util::maybe<Screen&> next) {
		_mailbox.disable();
	}

	void World_map_screen::_update(Time dt) {
		_mailbox.update_subscriptions();

		_ui_text.set("Test");
		// _engine.screens().enter<Loading_screen>(next_level);
	}


	void World_map_screen::_draw() {
		_ui_text.draw(_render_queue, glm::vec2(0), glm::vec4(1,1,1,1), 1.0f);

		_render_queue.flush();
	}
}
