/** Window & OpenGL-Context creation + management ****************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <SDL2/SDL.h>
#include <glm/vec3.hpp>

#include "../units.hpp"


namespace lux {
	namespace asset{
		class Asset_manager;
	}

namespace renderer {
	class Graphics_ctx {
		public:
			Graphics_ctx(const std::string& name, asset::Asset_manager& assets);
			~Graphics_ctx();

			void start_frame();
			void end_frame(Time delta_time);
			void set_clear_color(float r, float g, float b);

			void reset_viewport()const noexcept;

			auto win_width()const noexcept{return _win_width;}
			auto win_height()const noexcept{return _win_height;}
			auto viewport()const noexcept {return glm::vec4{0, 0, _win_width, _win_height};}
			auto gamma()const noexcept {return _gamma;}
			auto bloom()const noexcept {return _bloom;}
			auto supersampling()const noexcept {return _supersampling;}

			void settings(int width, int height, bool fullscreen, float gamma,
			              bool bloom, float supersampling);

		private:
			asset::Asset_manager& _assets;
			std::string _name;
			int _win_width, _win_height;
			bool _fullscreen;
			float _gamma;
			float _supersampling;
			bool _bloom;

			std::unique_ptr<SDL_Window,void(*)(SDL_Window*)> _window;
			SDL_GLContext _gl_ctx;
			glm::vec3 _clear_color;
			bool _clear_color_dirty = true;

			float _frame_start_time = 0;
			float _delta_time_smoothed = 0;
			float _cpu_delta_time_smoothed = 0;
			float _time_since_last_FPS_output = 0;
	};

	struct Disable_depthtest {
		Disable_depthtest();
		~Disable_depthtest();
	};
	struct Disable_depthwrite {
		Disable_depthwrite();
		~Disable_depthwrite();
	};
}
}

