/** a very very stupidly simple button ***************************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "widget.hpp"

#include "../renderer/primitives.hpp"
#include "../renderer/text.hpp"
#include "../audio/audio_ctx.hpp"


namespace lux {
namespace gui {

	class Button : public Widget {
		public:
			Button(Ui_ctx& ctx, std::string label,
			       Listerner clicked=Listerner{},
			       Listerner focused=Listerner{});

			void draw  (bool active)override;

			void on_activate  ()override;

			auto size()const noexcept -> glm::vec2 override;

		private:
			Ui_texture   _border;
			Ui_text      _label;

			Listerner    _clicked;
			Listerner    _focused;
			bool         _last_active = false;
	};


}
}
