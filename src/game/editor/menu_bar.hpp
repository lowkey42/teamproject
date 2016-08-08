/** The top menu bar for the actions *****************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "editor_comp.hpp"

#include "selection.hpp"

#include <core/input/input_manager.hpp>
#include <core/renderer/text.hpp>
#include <core/utils/maybe.hpp>


namespace lux {
namespace editor {

	// TODO: tool_tip
	class Menu_bar {
		public:
			Menu_bar(Engine&, asset::Asset_manager& assets, renderer::Camera_2d& camera_ui);

			void add_action(util::Str_id name, asset::AID icon, std::string tooltip,
			                std::function<void()>, std::function<bool()> enabled={});
			void add_action(util::Str_id name, asset::AID icon, bool def_state, std::string tooltip,
			                std::function<void(bool)>, std::function<bool()> enabled={});

			void disable_action(util::Str_id name);
			void enable_action(util::Str_id name);
			void force_toggle_state(util::Str_id name, bool state);

			void draw(renderer::Command_queue& queue);
			void update(Time dt);

			// true = mouse-input has been used
			auto handle_pointer(util::maybe<glm::vec2> mp1,
			                    util::maybe<glm::vec2> mp2) -> bool;

			void toggle_input(bool enable);

		private:
			struct Action {
				util::Str_id              name;
				renderer::Texture_ptr     icon;
				std::string               tooltip;
				bool                      enabled_override = true;
				bool                      toggle_state = false;
				std::function<bool()>     enabled;
				std::function<void()>     callback;
				std::function<void(bool)> callback_toggle;

				Action(util::Str_id, renderer::Texture_ptr, std::string, bool, std::function<bool()>,
				       std::function<void()>, std::function<void(bool)>);

				void call();
			};

			util::Mailbox_collection _mailbox;
			asset::Asset_manager&    _assets;
			renderer::Camera_2d&     _camera_ui;
			input::Input_manager&    _input_manager;
			renderer::Texture_ptr    _bg_left;
			renderer::Texture_ptr    _bg_center;
			renderer::Texture_ptr    _bg_right;
			std::vector<Action>      _actions;

			glm::vec2                       _tooltip_pos;
			Time                            _tooltip_delay_left{};
			renderer::Text_dynamic          _tooltip_text;
			mutable renderer::Texture_batch _batch;

			auto _find_by_position(glm::vec2 mouse_screen_pos) -> util::maybe<Action&>;
			auto _find_by_name(util::Str_id name) -> util::maybe<Action&>;

			auto _calc_y_offset()const -> float;
			auto _calc_menu_width()const -> float;

			// ([0-1], [0-N]); N=_actions.size()
			auto _to_local_offset(glm::vec2 p)const -> glm::vec2;

			auto _last_mouse_screen_pos()const -> glm::vec2;

	};

}
}
