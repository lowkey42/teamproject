#include "widget.hpp"

namespace lux {
namespace gui {

	Widget::Widget(Engine& engine, util::maybe<Widget&> parent, Layout_desc layout)
	    : _engine(engine), _parent(parent), _layout(layout) {
	}

	void Widget::draw(Draw_ctx ctx) {
		_on_draw(ctx);

		for(auto& c : _children) {
			c->draw(ctx);
		}
	}

	void Widget::update(Time dt) {
		_on_update(dt);

		for(auto& c : _children) {
			c->update(dt);
		}
	}

	void Widget::focus(glm::vec2 position) {
		_refocus( std::find_if(_children.begin(), _children.end(), [&](auto& c){
			return c->_inside(position);
		}) );

		if(_focused_child!=_children.end()) {
			(*_focused_child)->focus(position);
		}
	}
	void Widget::focus_next(glm::vec2 dir) {
		_refocus(_children.end()); // TODO: find next focus

		if(_focused_child!=_children.end()) {
			(*_focused_child)->focus_next(dir);
		}
	}
	void Widget::_refocus(Child_iter new_focus) {
		_on_focus();

		if(_focused_child!=new_focus) {
			if(_focused_child!=_children.end()) {
				(*_focused_child)->unfocus();
			}

			_focused_child = new_focus;
		}
	}

	void Widget::unfocus() {
		_on_unfocus();

		if(_focused_child!=_children.end()) {
			(*_focused_child)->unfocus();
		}
	}

	void Widget::click(util::maybe<glm::vec2> position) {
		_on_click(position);

		if(_focused_child!=_children.end()) {
			(*_focused_child)->click(position);
		}
	}

	void Widget::drag_start(glm::vec2 position) {
		_on_drag_start(position);

		_drag_focus = _focused_child;

		if(_focused_child!=_children.end()) {
			(*_focused_child)->drag_start(position);
		}
	}

	void Widget::drag_end(glm::vec2 position) {
		_on_drag_end(position);

		if(_focused_child!=_children.end()) {
			(*_focused_child)->drag_end(position);
		}
		if(_drag_focus!=_children.end() && _drag_focus!=_focused_child) {
			(*_drag_focus)->drag_end(position);
		}
	}

	void Widget::update_layout(glm::vec2 center, glm::vec2 parent_size) {
		auto size = parent_size;
		size.x = _layout.size_percent.x > 0 ? size.x * _layout.size_percent.x
		                                    : size.y * _layout.size_percent.y * _layout.aspect_ratio;

		size.y = _layout.size_percent.y > 0 ? size.y * _layout.size_percent.y
		                                    : size.x / _layout.aspect_ratio;

		_global_center = center;
		_global_size = size;

		_on_layout_updated();

		for(auto& c : _children) {
			auto child_pos = center; // TODO

			c->update_layout(child_pos, size);
		}
	}

}
}
