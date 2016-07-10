/** Manager for nuklear ui ***************************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "../renderer/command_queue.hpp"


struct nk_context;


namespace lux {
	class Engine;

namespace gui {

	class Gui {
		public:
			Gui(Engine& engine);

			void draw(renderer::Command_queue&);

			auto ctx() -> nk_context*;


		private:
			struct PImpl;
			std::unique_ptr<PImpl> _impl;
	};



}
}
