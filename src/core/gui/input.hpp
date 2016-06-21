/** a very very stupidly simple inputbox *************************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "widget.hpp"


namespace lux {
namespace gui {
/*
	using Input_listerner = std::function<void(std::string)>;

	class Input : public Widget {
		public:
			Input(Ui_ctx& ctx,
			       Input_listerner finished, int32_t max=-1,
			       Listerner focused=Listerner{});

			void draw  (bool active)override;

			void on_input(const std::string& c)override;

			auto size()const noexcept -> glm::vec2 override;

		private:
			Ui_texture   _border;
			Ui_text      _text;
			std::string  _text_str;

			Input_listerner    _finished;
			Listerner    _focused;
			int32_t      _max;
			bool         _last_active = false;
	};
*/

}
}
