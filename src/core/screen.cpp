#include "screen.hpp"

#include "engine.hpp"


namespace mo {

	Screen_manager::Screen_manager(Engine& engine)
	  : _engine(engine) {
	}

	Screen_manager::~Screen_manager() noexcept {
		clear();
	}

	auto Screen_manager::enter(std::unique_ptr<Screen> screen) -> Screen& {
		if(!_screen_stack.empty())
			_screen_stack.back()->_on_leave(*screen);

		screen->_on_enter(util::justPtr(_screen_stack.empty() ? nullptr : _screen_stack.back().get()));

		if(screen->_prev_screen_policy()==Prev_screen_policy::discard)
			_screen_stack.clear();

		_screen_stack.push_back(std::move(screen));

		return *_screen_stack.back().get();
	}

	void Screen_manager::leave(uint8_t depth) {
		if(depth<=0) return;

		auto last = std::shared_ptr<Screen>{};
		if(!_screen_stack.empty())
			last = _screen_stack.back();

		for(int32_t i = std::min(std::size_t(depth), _screen_stack.size()); i>0; i--)
			_screen_stack.pop_back();

		if(_screen_stack.empty()) {
			_engine.exit();
			if(last)
				last->_on_leave(util::nothing());

		} else if(last) {
			last->_on_leave(util::justPtr(_screen_stack.back().get()));
			_screen_stack.back()->_on_enter(util::justPtr(last.get()));
		} else {
			_screen_stack.back()->_on_enter(util::nothing());
		}
	}

	auto Screen_manager::current() -> Screen& {
		return *_screen_stack.back();
	}

	void Screen_manager::on_frame(Time delta_time) {
		auto screen_stack = _screen_stack;

		const int screen_stack_size = screen_stack.size();
		int screen_index=screen_stack_size-1; //< has to be signed

		// update all screens until we reached one with PrevScreenPolicy < Update
		for(; screen_index>=0; screen_index--) {
			auto& s = screen_stack.at(screen_index);

			s->_update(delta_time);

			if(s->_prev_screen_policy()!=Prev_screen_policy::update)
				break;
		}

		// find last screen to draw (PrevScreenPolicy >= Draw)
		for(; screen_index>=0; screen_index--)
			if(screen_stack.at(screen_index)->_prev_screen_policy()!=Prev_screen_policy::draw)
				break;

		screen_index=std::max(screen_index, 0);

		// draw all screens in reverse order
		for(; screen_index<screen_stack_size; screen_index++)
			screen_stack.at(screen_index)->_draw();
	}

	void Screen_manager::clear() {
		// unwind screen-stack
		for(auto& s : _screen_stack)
			s->_on_leave(util::nothing());

		_screen_stack.clear();
	}

}
