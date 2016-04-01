#include "engine.hpp"

#include "asset/asset_manager.hpp"
#include "audio/audio_ctx.hpp"
#include "audio/sound.hpp"
#include "utils/log.hpp"
#include "utils/rest.hpp"
#include "input/input_manager.hpp"
#include "renderer/graphics_ctx.hpp"

#include <stdexcept>


namespace lux {
namespace {
	void init_sub_system(Uint32 f, const std::string& name, bool required=true) {
		if(SDL_InitSubSystem(f)!=0) {
			auto m = "Could not initialize "+name+": "+get_sdl_error();

			if(required)
				FAIL(m);
			else
				WARN(m);
		}
	}
}

	using namespace unit_literals;

	auto get_sdl_error() -> std::string {
		std::string sdl_error(SDL_GetError());
		SDL_ClearError();
		return sdl_error;
	}

	Engine::Sdl_wrapper::Sdl_wrapper() {
		INVARIANT(SDL_Init(0)==0, "Could not initialize SDL: "<<get_sdl_error());

		init_sub_system(SDL_INIT_AUDIO, "SDL_Audio");
		init_sub_system(SDL_INIT_VIDEO, "SDL_Video");
		init_sub_system(SDL_INIT_JOYSTICK, "SDL_Joystick");
		init_sub_system(SDL_INIT_HAPTIC, "SDL_Haptic", false);
		init_sub_system(SDL_INIT_GAMECONTROLLER, "SDL_Gamecontroller");
		init_sub_system(SDL_INIT_EVENTS, "SDL_Events");
	}
	Engine::Sdl_wrapper::~Sdl_wrapper() {
		SDL_Quit();
	}


	Engine::Engine(const std::string& title, int argc, char** argv, char** env)
	  : _screens(*this),
	    _asset_manager(std::make_unique<asset::Asset_manager>(argc>0 ? argv[0] : "", title)),
	    _sdl(),
	    _graphics_ctx(std::make_unique<renderer::Graphics_ctx>(title, *_asset_manager)),
	    _audio_ctx(std::make_unique<audio::Audio_ctx>(*_asset_manager)),
	    _input_manager(std::make_unique<input::Input_manager>(_bus, *_asset_manager)),
	    _current_time(SDL_GetTicks() / 1000.0f) {

		_input_manager->viewport(_graphics_ctx->viewport());
	}

	Engine::~Engine() noexcept {
		_screens.clear();

		if(_audio_ctx) {
			_audio_ctx->stop_music();
			_audio_ctx->pause_sounds();
		}

		assets().shrink_to_fit();
	}

	void Engine::on_frame() {
		_last_time = _current_time;
		_current_time = SDL_GetTicks() / 1000.0f;
		const auto delta_time = std::min(_current_time - _last_time, 1.f) * second;

		_graphics_ctx->start_frame();

		util::rest::update();
		_bus.update();

		if(_audio_ctx) {
			_audio_ctx->flip();
		}

		_input_manager->update(delta_time);

		_on_frame(delta_time);

		_poll_events();

		_screens.on_frame(delta_time);

		_graphics_ctx->end_frame(delta_time);
	}

	void Engine::_poll_events() {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				_quit = true;
			else
				_input_manager->handle_event(event);

			if(event.type==SDL_KEYDOWN) {
				if(event.key.keysym.sym==SDLK_F12)
					assets().reload();
			}
		}
	}

}
