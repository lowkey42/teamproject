/**************************************************************************\
 * implementation of command pattern (e.g. for undo/redo)                 *
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

#include "log.hpp"
#include <range/v3/all.hpp>

#include <string>
#include <stdexcept>
#include <iosfwd>
#include <cmath>
#include <algorithm>

namespace mo {
namespace util {

	struct Command {
		virtual ~Command() = default;

		virtual void execute() = 0;
		virtual void undo() = 0;
		virtual auto name()const -> const std::string&;
	};

	class Command_manager {
		private:
			std::vector<std::unique_ptr<Command>> _commands;
			std::size_t _history_size;

			auto _command_names()const {
				return _commands | ranges::view::transform([](auto& cmd){return cmd->name();});
			}

		public:
			void execute(std::unique_ptr<Command> cmd);

			template<class T, class... Args>
			void execute(Args&&... args) {
				execute(std::make_unique<T>(std::forward<Args>(args)...));
			}

			void undo();
			void redo();

			auto history()const {
				return _command_names() | ranges::view::take(_history_size);
			}
			auto future()const {
				return _command_names() | ranges::view::drop(_history_size);
			}

			auto undo_available()const -> bool;
			auto redo_available()const -> bool;
	};


}
}
