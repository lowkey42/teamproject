/** An overlay window for modifications of level properties ******************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "../level.hpp"
#include "../meta_system.hpp"

#include <core/gui/translations.hpp>

namespace lux {
namespace gui {class Translator; class Gui;}
namespace editor {

	class Level_settings {
		public:
			Level_settings(gui::Gui&, const gui::Translator&, Meta_system&);
			~Level_settings();

			void update_and_draw(Level_info&);

			void visible(bool v) {_visible = v;}
			auto visible()const {return _visible;}

		private:
			struct PImpl;
			std::unique_ptr<PImpl> _impl;

			gui::Gui& _gui;
			const gui::Translator& _translator;
			Meta_system& _systems;
			bool _visible = false;
	};

}
}
