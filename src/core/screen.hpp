/**************************************************************************\
 * manages the screens of the application (game, menues, etc.)            *
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

#include "units.hpp"
#include "utils/maybe.hpp"

#include <vector>
#include <memory>


namespace mo {
	class Engine;

	enum class Prev_screen_policy {
		discard,
		stack,
		draw,
		update
	};

	class Screen {
		public:
			Screen(Engine& engine) : _engine(engine) {}
			Screen(const Screen&) = delete;
			Screen& operator=(const Screen&) = delete;

			virtual ~Screen()noexcept = default;

		protected:
			friend class Screen_manager;

			virtual void _on_enter(util::maybe<Screen&> prev){}
			virtual void _on_leave(util::maybe<Screen&> next){}
			virtual void _update(Time delta_time) = 0;
			virtual void _draw() = 0;
			virtual auto _prev_screen_policy()const noexcept -> Prev_screen_policy = 0;

			Engine& _engine;
	};

	class Screen_manager {
		friend class Screen;
		public:
			Screen_manager(Engine& engine);
			~Screen_manager() noexcept;

			template<class T, typename ...Args>
			auto enter(Args&&... args) -> T& {
				return static_cast<T&>(enter(std::make_unique<T>(_engine, std::forward<Args>(args)...)));
			}

			auto enter(std::unique_ptr<Screen> screen) -> Screen&;
			void leave(uint8_t depth=1);
			auto current() -> Screen&;

			void on_frame(Time delta_time);
			void clear();

		protected:
			Engine& _engine;
			std::vector<std::shared_ptr<Screen>> _screen_stack;
	};

}
