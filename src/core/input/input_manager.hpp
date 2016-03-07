/**************************************************************************\
 * initialization & event handling of input devices                       *
 *                                               ___                      *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2014 Florian Oetke                                       *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include "types.hpp"
#include "events.hpp"

#include "../units.hpp"
#include "../utils/messagebus.hpp"
#include "../utils/str_id.hpp"

#include <glm/vec2.hpp>
#include <SDL2/SDL.h>
#include <memory>
#include <unordered_map>


namespace mo {
	namespace asset {
		class Asset_manager;
	}

namespace input {

	class Input_mapper;

	class Input_manager {
		private:
			static constexpr auto _max_pointers = 2;
		public:
			Input_manager(util::Message_bus& bus, asset::Asset_manager&);
			Input_manager(const Input_manager&) = delete;
			Input_manager(Input_manager&&) = delete;
			~Input_manager()noexcept;

			void update(Time dt);
			void handle_event(SDL_Event& event);

			void screen_to_world_coords(std::function<glm::vec2(glm::vec2)> func) {
				_screen_to_world_coords = func;
			}
			void viewport(glm::vec4 v) {
				_viewport = v;
			}

			auto last_pointer_world_position(int idx=0)const noexcept {
				return _pointer_world_pos[idx];
			}
			auto last_pointer_screen_position(int idx=0)const noexcept {
				return _pointer_screen_pos[idx];
			}

			auto pointer_world_position(int idx=0)const noexcept {
				return _pointer_active[idx] ? util::justCopy(_pointer_world_pos[idx]) : util::nothing();
			}
			auto pointer_screen_position(int idx=0)const noexcept {
				return _pointer_active[idx] ? util::justCopy(_pointer_screen_pos[idx]) : util::nothing();
			}

			void enable_context(Context_id id);

		private:
			void _add_gamepad(int joystick_id);
			void _remove_gamepad(int instance_id);

		private:
			class Gamepad;

			util::Mailbox_collection _mailbox;

			glm::vec4 _viewport;
			std::function<glm::vec2(glm::vec2)> _screen_to_world_coords;
			std::vector<std::unique_ptr<Gamepad>> _gamepads;

			std::array<glm::vec2, _max_pointers> _pointer_screen_pos{};
			std::array<glm::vec2, _max_pointers> _pointer_world_pos{};
			std::array<bool,      _max_pointers> _pointer_active{};
			std::array<int64_t,   _max_pointers> _pointer_finger_id{};

			std::unique_ptr<Input_mapper> _mapper;
	};

}
}
