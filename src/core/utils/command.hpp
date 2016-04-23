/** implementation of command pattern (e.g. for undo/redo) *******************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "log.hpp"

#include <string>
#include <vector>
#include <memory>


namespace lux {
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

			void clear() {_commands.clear(); _history_size=0;}

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
