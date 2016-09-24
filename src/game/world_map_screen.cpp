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
	      _camera_ui(engine.graphics_ctx().viewport(), calculate_vscreen(engine, 1080)),
	      _level_pack(get_level_pack(engine,level_pack_id))
	{

		unlock_level(_engine, _level_pack->level_ids.front().aid);

		_mailbox.subscribe_to([&](input::Once_action& e){
			switch(e.id) {
				case "quit"_strid:
					_engine.screens().leave();
					break;

				case "select_a"_strid:
					_enter_nth_level(0);
					break;
				case "select_b"_strid:
					_enter_nth_level(1);
					break;
				case "select_c"_strid:
					_enter_nth_level(2);
					break;
				case "select_d"_strid:
					_enter_nth_level(3);
					break;
				case "select_e"_strid:
					_enter_nth_level(4);
					break;
				case "select_f"_strid:
					_enter_nth_level(5);
					break;
				case "select_g"_strid:
					_enter_nth_level(6);
					break;
				case "select_h"_strid:
					_enter_nth_level(7);
					break;
				case "select_i"_strid:
					_enter_nth_level(8);
					break;
			}
		});

		_render_queue.shared_uniforms(renderer::make_uniform_map("vp", _camera_ui.vp()));
	}

	void World_map_screen::_enter_nth_level(std::size_t idx) {
		if(idx >= _level_pack->level_ids.size())
			return;

		auto id = _level_pack->level_ids.at(idx).aid;

		if(!is_level_locked(_engine,id))
			_engine.screens().enter<Game_screen>(id, true);
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

		std::stringstream ss;
		ss<<"Levels in "<<_level_pack->name<<":\n";

		for(auto i=0u; i<_level_pack->level_ids.size(); i++) {
			auto id = _level_pack->level_ids.at(i).aid;
			ss<<"  "<<(i+1)<<". "<<id<<" ("<<(is_level_locked(_engine,id) ? "locked" : "unlocked")<<")\n";
		}

		ss<<"\nPress the corresponding number to start a level.";

		_ui_text.set(ss.str());
	}


	void World_map_screen::_draw() {
		_ui_text.draw(_render_queue, glm::vec2(0,-400), glm::vec4(1,1,1,1), 1.0f);

		_render_queue.flush();
	}
}
