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

#include <string>
#include <vector>
#include <memory>


namespace mo {
namespace util {

	struct Command {
		Command() = default;
		virtual ~Command() = default;

		virtual void execute() = 0;
		virtual void undo() = 0;
		virtual auto name()const -> const std::string& = 0;
	};

	class Command_manager {
		public:
			void execute(std::unique_ptr<Command> cmd);

			template<class T, class... Args>
			void execute(Args&&... args) {
				execute(std::make_unique<T>(std::forward<Args>(args)...));
			}

			void undo();
			void redo();

			auto history()const -> std::vector<std::string>;
			auto future()const -> std::vector<std::string>;

			auto undo_available()const -> bool {return _history_size>0;}
			auto redo_available()const -> bool {return _history_size<_commands.size();}

		private:
			std::vector<std::unique_ptr<Command>> _commands;
			std::size_t _history_size = 0;
	};


}
}
