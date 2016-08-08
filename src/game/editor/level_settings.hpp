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

	// returns new visible state
	extern bool draw_settings(const gui::Translator&, gui::Gui&, bool visible,
	                          Level_info&, Meta_system&);

}
}
