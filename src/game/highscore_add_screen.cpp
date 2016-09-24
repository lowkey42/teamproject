#include "highscore_add_screen.hpp"

#include "../core/units.hpp"
#include "../core/renderer/graphics_ctx.hpp"

#include <core/audio/music.hpp>
#include <core/audio/audio_ctx.hpp>

#include <core/input/events.hpp>
#include <core/input/input_manager.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iomanip>
#include <sstream>


namespace lux {
	using namespace unit_literals;
	using namespace renderer;

	static constexpr auto max_player_name = 15;

	Highscore_add_screen::Highscore_add_screen(Engine& engine, std::string level, Time time)
	    : Screen(engine),
	      _mailbox(engine.bus()),
	      _highscores(dynamic_cast<Game_engine&>(engine).highscore_manager()),
	      _camera(engine.graphics_ctx().viewport(), calculate_vscreen(engine, 1080)),
	      _debug_Text(engine.assets().load<Font>("font:menu_font"_aid)),
	      _highscore_list(_highscores.get_highscore(level)),
	      _level_id(std::move(level)),
	      _time(time),
	      _player_name(_highscores.last_username())
	{

		_mailbox.subscribe_to([&](input::Once_action& e){
			switch(e.id) {
				case "quit"_strid:
					_engine.screens().leave(1);
					break;

				case "continue"_strid:
					_engine.screens().leave(2);
					break;

				case "backspace"_strid:
					if(!_player_name.empty()) {
						_player_name.pop_back();
					}
					break;
			}
		}, [&](input::Char_input& c) {
			if(_player_name.size()<max_player_name) {
				_player_name += c.character;
			}
		});

		_render_queue.shared_uniforms(renderer::make_uniform_map("VP", _camera.vp()));
	}

	void Highscore_add_screen::_on_enter(util::maybe<Screen&> prev) {
		_engine.input().enable_context("menu"_strid);
		_mailbox.enable();

		//_engine.audio_ctx().play_music(_engine.assets().load<audio::Music>("music:intro"_aid));
	}

	void Highscore_add_screen::_on_leave(util::maybe<Screen&> next) {
		_mailbox.disable();

		if(!_player_name.empty()) {
			_highscores.push_highscore(_level_id, Highscore{_player_name, _time/1_s});
		}
	}

	namespace {
		void print_score(std::stringstream& ss, int rank, const std::string& name, float time) {
			ss<<" "<<std::setw(2)<<std::right<<rank
			  <<". "<<std::setw(max_player_name+4)<<std::internal<<name<<" | "
			  <<std::setw(6)<<std::right<< std::fixed << std::setprecision(1)<<time<<"\n";
		}
	}

	void Highscore_add_screen::_update(Time) {
		_mailbox.update_subscriptions();

		if(_highscore_list || _highscore_list.try_load()) {
			auto rank = _highscore_list->get_rank(_time);

			if(rank>=10) {
				_engine.screens().leave();

			} else {
				std::stringstream ss;
				ss << "New high score at rank "<<rank<<"\n";

				int i=0;
				for(auto& score : _highscore_list->scores) {
					if(i++==rank) {
						print_score(ss, rank+1, "* "+_player_name+" *", _time.value());
						i++;
					}

					print_score(ss, i, score.name, score.time);
				}
				if(i==rank) {
					print_score(ss, rank+1, _player_name, _time.value());
				}

				ss<<"\nEnter your name and press return to continue or escape to retry...";

				_debug_Text.set(ss.str(), true);

			}

		} else {
			_debug_Text.set("loading highscore ...");
		}
	}


	void Highscore_add_screen::_draw() {
		_debug_Text.draw(_render_queue,  glm::vec2(0,0), glm::vec4(1,1,1,1), 0.5f);

		_render_queue.flush();
	}

}
