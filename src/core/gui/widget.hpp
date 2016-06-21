/** base classes for the simple game ui **************************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "../utils/messagebus.hpp"
#include "../utils/maybe.hpp"
#include "../units.hpp"

#include <glm/vec2.hpp>

#include <string>
#include <vector>
#include <memory>


namespace lux {
	class Engine;

namespace renderer {
	class Command_queue;
	class Texture_batch;
}

namespace gui {


	enum class Anchor {
		top_left,    top_center,    top_right,
		middle_left, middle_center, middle_right,
		bottom_left, bottom_center, bottom_right
	};

	struct Layout_desc {
		glm::vec2 size_percent {1,1};
		Anchor    anchor = Anchor::middle_center;
		float     margin_top = 0.f;
		float     margin_left = 0.f;
		float     margin_right = 0.f;
		float     margin_bottom = 0.f;
		bool      floating = false; //< ignore widget in parent layout
		float     aspect_ratio = 1.f;
	};

	struct Draw_ctx {
		renderer::Command_queue& queue;
		renderer::Texture_batch& batch;
	};


	class Widget {
		public:
			Widget(Engine& engine, util::maybe<Widget&> parent, Layout_desc layout={});
			virtual ~Widget() = default;

			void draw  (Draw_ctx ctx);
			void update(Time dt);

			void focus(glm::vec2 position);
			void focus_next(glm::vec2 dir);
			void unfocus();
			void click(util::maybe<glm::vec2> position);
			void drag_start(glm::vec2 position);
			void drag_end(glm::vec2 global_position);

			template<class W, class... Args>
			auto add_child(Args&&... args) -> W& {
				auto c = std::make_unique<W>(_engine, *this, std::forward<Args>(args)...);
				auto& c_ref = *c;
				_children.push_back(std::move(c));
				return c_ref;
			}

			void update_layout(glm::vec2 center, glm::vec2 parent_size);

		protected:
			virtual void _on_update(Time dt) {}
			virtual void _on_draw(Draw_ctx ctx)const {}
			virtual void _on_focus() {}
			virtual void _on_unfocus() {}
			virtual void _on_click(util::maybe<glm::vec2> position) {}
			virtual void _on_drag_start(glm::vec2 position) {}
			virtual void _on_drag_end(glm::vec2 position) {}
			virtual void _on_layout_updated() {}

			auto _center()const noexcept {return _global_center;}
			auto _size()const noexcept {return _global_size;}
			auto _inside(glm::vec2 pos)const noexcept {
				return pos.x>=_global_center.x-_global_size.x/2.f &&
				       pos.y>=_global_center.y-_global_size.y/2.f &&
				       pos.x>=_global_center.x+_global_size.x/2.f&&
				       pos.y>=_global_center.y+_global_size.y/2.f;
			}

		private:
			using Child_iter = std::vector<std::unique_ptr<Widget>>::iterator;

			Engine& _engine;
			util::maybe<Widget&> _parent;
			std::vector<std::unique_ptr<Widget>> _children;
			Child_iter _focused_child;
			Child_iter _drag_focus;

			Layout_desc _layout;
			glm::vec2 _global_center;
			glm::vec2 _global_size;

			void _refocus(Child_iter new_focus);
	};

}
}
