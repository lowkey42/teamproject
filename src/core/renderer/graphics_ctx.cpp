#ifndef ANDROID
	#include <GL/glew.h>
	#include <GL/gl.h>
#else
	#include <GLES2/gl2.h>
#endif

#include "graphics_ctx.hpp"

#include "text.hpp"
#include "sprite_batch.hpp"
#include "texture_batch.hpp"
#include "material.hpp"
#include "primitives.hpp"

#include "../utils/log.hpp"
#include "../asset/asset_manager.hpp"

#include <sf2/sf2.hpp>

#include <iostream>
#include <sstream>
#include <cstdio>


namespace lux {
namespace renderer {

	using namespace unit_literals;

	namespace {
		void sdl_error_check() {
			const char *err = SDL_GetError();
			if(*err != '\0') {
				std::string errorStr(err);
				SDL_ClearError();
				FAIL("SDL: "<<errorStr);
			}
		}

#ifndef ANDROID
	#ifndef EMSCRIPTEN
		void
	#ifdef GLAPIENTRY
		GLAPIENTRY
	#endif
		gl_debug_callback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void*) {
			if(severity!=GL_DEBUG_SEVERITY_LOW
			   && severity!=GL_DEBUG_SEVERITY_MEDIUM
			   && severity!=GL_DEBUG_SEVERITY_HIGH)
				return;

			auto& log = [&]() -> auto& {
				switch (severity){
					case GL_DEBUG_SEVERITY_LOW:
					case GL_DEBUG_SEVERITY_MEDIUM:
						return util::warn("", "", 0);

					case GL_DEBUG_SEVERITY_HIGH:
						return util::error("", "", 0);
					default:
						return util::info("", "", 0);
				}
			}();
			log<<"[GL] ";
			// TODO: source


			switch (type) {
				case GL_DEBUG_TYPE_ERROR:
					log << "ERROR";
					break;
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
					log << "DEPRECATED_BEHAVIOR";
					break;
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
					log << "UNDEFINED_BEHAVIOR";
					break;
				case GL_DEBUG_TYPE_PORTABILITY:
					log << "PORTABILITY";
					break;
				case GL_DEBUG_TYPE_PERFORMANCE:
					log << "PERFORMANCE";
					break;
				case GL_DEBUG_TYPE_OTHER:
					log << "OTHER";
					break;
			}

			log<<"("<<id<<"): "<<message;

			log<<std::endl;
		}
	#endif
#endif

		struct Graphics_cfg {
			int width;
			int height;
			bool fullscreen;
			float max_screenshake;
			float brightness;

			Graphics_cfg();
		};

		sf2_structDef(Graphics_cfg,
			width,
			height,
			fullscreen,
			max_screenshake,
			brightness
		)

	#ifndef EMSCRIPTEN
		Graphics_cfg::Graphics_cfg() : width(1920), height(1080), fullscreen(true),
		    max_screenshake(0.5f), brightness(1.1f) {}
	#else
		Graphics_cfg::Graphics_cfg() : width(1024), height(512), fullscreen(false),
		    max_screenshake(0.5f), brightness(1.2f) {}
	#endif

	}


	Graphics_ctx::Graphics_ctx(const std::string& name, asset::Asset_manager& assets)
	 : _assets(assets), _name(name), _window(nullptr, SDL_DestroyWindow) {

		auto default_cfg = Graphics_cfg{};

		auto& cfg = asset::unpack(assets.load_maybe<Graphics_cfg>("cfg:graphics"_aid)).get_or_other(
			default_cfg
		);

		_win_width = cfg.width;
		_win_height = cfg.height;
		_max_screenshake = cfg.max_screenshake;
		_brightness = cfg.brightness;
		_fullscreen = cfg.fullscreen;

		if(&cfg==&default_cfg) {
			assets.save<Graphics_cfg>("cfg:graphics"_aid, cfg);
		}

#ifndef EMSCRIPTEN
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		int win_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
		if(_fullscreen)
			win_flags|=SDL_WINDOW_FULLSCREEN_DESKTOP;

		_window.reset( SDL_CreateWindow(_name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							_win_width, _win_height, win_flags) );

		if (!_window)
			FAIL("Unable to create window");

		sdl_error_check();

		SDL_GetWindowSize(_window.get(), &_win_width, &_win_height);

		try {
			_gl_ctx = SDL_GL_CreateContext(_window.get());
			sdl_error_check();
			SDL_GL_MakeCurrent(_window.get(), _gl_ctx);
			sdl_error_check();

		} catch (const std::runtime_error& ex) {
			FAIL("Failure to create OpenGL context. This application requires a OpenGL ES 2.0 capable GPU. Error was: "<< ex.what());
		}

		if(SDL_GL_SetSwapInterval(-1)) SDL_GL_SetSwapInterval(1);

#ifndef ANDROID
		glewExperimental = GL_TRUE;
		glewInit();

	#ifndef EMSCRIPTEN
		if(GLEW_KHR_debug){
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback((GLDEBUGPROC)gl_debug_callback, stderr);
		}
		else{
			WARN("No OpenGL debug log available.");
		}
	#endif
#endif

		glLineWidth(2.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		set_clear_color(0.0f,0.0f,0.0f);

		init_font_renderer(assets);
		init_sprite_renderer(assets);
		init_texture_renderer(assets);
		init_materials(assets);
		init_primitives(assets);
	}

	Graphics_ctx::~Graphics_ctx() {
		SDL_GL_DeleteContext(_gl_ctx);
	}

	void Graphics_ctx::reset_viewport()const noexcept {
		glViewport(0,0, win_width(), win_height());
	}

	void Graphics_ctx::start_frame() {
		if(_clear_color_dirty) {
			glClearColor(_clear_color.r, _clear_color.g, _clear_color.b,1.f);
			_clear_color_dirty = false;
		}

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		_frame_start_time = SDL_GetTicks() / 1000.0f;
	}
	void Graphics_ctx::end_frame(Time delta_time) {
		float smooth_factor=0.1f;
		_delta_time_smoothed=(1.0f-smooth_factor)*_delta_time_smoothed+smooth_factor*delta_time/second;

		float cpu_delta_time = SDL_GetTicks() / 1000.0f - _frame_start_time;
		_cpu_delta_time_smoothed=(1.0f-smooth_factor)*_cpu_delta_time_smoothed+smooth_factor*cpu_delta_time;

		_time_since_last_FPS_output+=delta_time/second;
		if(_time_since_last_FPS_output>=1.0f){
			_time_since_last_FPS_output=0.0f;
			std::ostringstream osstr;
			osstr<<_name<<" ("<<(int((1.0f/_delta_time_smoothed)*10.0f)/10.0f)<<" FPS, ";
			osstr<<(int(_delta_time_smoothed*10000.0f)/10.0f)<<" ms/frame, ";
			osstr<<(int(_cpu_delta_time_smoothed*10000.0f)/10.0f)<<" ms/frame [cpu])";

#ifdef EMSCRIPTEN
			DEBUG(_cpu_delta_time_smoothed);
#else
			SDL_SetWindowTitle(_window.get(), osstr.str().c_str());
#endif
		}
		SDL_GL_SwapWindow(_window.get());

		// unbind texture
//		glActiveTexture(0);
//		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void Graphics_ctx::set_clear_color(float r, float g, float b) {
		_clear_color = glm::vec3(r,g,b);
		_clear_color_dirty = true;
	}

	auto Graphics_ctx::max_screenshake()const noexcept -> float {
		return _screenshake_enabled ? _max_screenshake * 100 : 0;
	}
	void Graphics_ctx::toggle_screenschake(bool enable) {
		_screenshake_enabled = enable;
	}

	void Graphics_ctx::resolution(int width, int height, float max_screenshake) {
		auto cfg = *_assets.load<Graphics_cfg>("cfg:graphics"_aid);
		cfg.width = width;
		cfg.height = height;
		cfg.max_screenshake = max_screenshake;
		_assets.save<Graphics_cfg>("cfg:graphics"_aid, cfg);
	}

	Disable_depthtest::Disable_depthtest() {
		glDisable(GL_DEPTH_TEST);
	}
	Disable_depthtest::~Disable_depthtest() {
		glEnable(GL_DEPTH_TEST);
	}

	Disable_depthwrite::Disable_depthwrite() {
		glDepthMask(GL_FALSE);
	}
	Disable_depthwrite::~Disable_depthwrite() {
		glDepthMask(GL_TRUE);
	}
}
}
