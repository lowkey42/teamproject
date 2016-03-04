#define GLM_SWIZZLE

#include "selection.hpp"

#include "editor_comp.hpp"

#include "../physics/transform_comp.hpp"

#include <core/input/input_manager.hpp>
#include <core/renderer/primitives.hpp>
#include <core/renderer/command_queue.hpp>


namespace mo {
namespace sys {
namespace editor {

	using namespace renderer;
	using namespace glm;

	Selection::Selection(Engine& engine, renderer::Camera& world_cam, util::Command_manager& commands)
	    : _mailbox(engine.bus()), _world_cam(world_cam), _commands(commands),
	      _input_manager(engine.input()),
	      _last_primary_pointer_pos(util::nothing()),
	      _last_secondary_pointer_pos(util::nothing()) {

		_icon_layer  = engine.assets().load<Texture>("tex:selection_icon_layer"_aid);
		_icon_move   = engine.assets().load<Texture>("tex:selection_icon_move"_aid);
		_icon_rotate = engine.assets().load<Texture>("tex:selection_icon_rotate"_aid);
		_icon_scale  = engine.assets().load<Texture>("tex:selection_icon_scale"_aid);


		_mailbox.subscribe_to([&](input::Continuous_action& e) {
			switch(e.id) {
				case "mouse_down"_strid:
					if(e.begin) {
						// TODO: begin action
					}
					break;
			}

		});
		_mailbox.subscribe_to([&](input::Once_action& e) {
			switch(e.id) {
				case "mouse_click"_strid: {
					auto mp = _input_manager.last_pointer_screen_position();
					_change_selection(mp);
					break;
				}
			}
		});
	}

	void Selection::draw(renderer::Command_queue& queue, renderer::Camera& cam) {
		if(_selected_entity) {
			auto& transform = _selected_entity->get<physics::Transform_comp>().get_or_throw();
			auto editor = _selected_entity->get<Editor_comp>();

			if(false /* TODO: check if this is a smart_texture */) {
				// TODO: draw edges and nodes

			} else if(editor.is_some()) {
				auto center = remove_units(transform.position());
				auto bounds = remove_units(editor.get_or_throw().bounds());

				auto top_left     = vec2{-bounds.x/2.f,  bounds.y/2.f};
				auto bottom_right = top_left*-1.f;
				auto top_right    = glm::vec2{bottom_right.x, top_left.y};
				auto bottom_left  = glm::vec2{top_left.x, bottom_right.y};

				top_left     = rotate(top_left, transform.rotation())     + center.xy();
				bottom_right = rotate(bottom_right, transform.rotation()) + center.xy();
				top_right    = rotate(top_right, transform.rotation())    + center.xy();
				bottom_left  = rotate(bottom_left, transform.rotation())  + center.xy();

				// world to screen
				top_left     = _world_cam.world_to_screen(vec3(top_left, center.z));
				bottom_right = _world_cam.world_to_screen(vec3(bottom_right, center.z));
				top_right    = _world_cam.world_to_screen(vec3(top_right, center.z));
				bottom_left  = _world_cam.world_to_screen(vec3(bottom_left, center.z));

				// screen to menu
				top_left     = cam.screen_to_world(top_left).xy();
				bottom_right = cam.screen_to_world(bottom_right).xy();
				top_right    = cam.screen_to_world(top_right).xy();
				bottom_left  = cam.screen_to_world(bottom_left).xy();

				draw_dashed_line(queue, top_left,     top_right,    2.0f, Rgba{1.f,0.8f, 0.25f, 1.0f});
				draw_dashed_line(queue, top_right,    bottom_right, 2.0f, Rgba{1.f,0.8f, 0.25f, 1.0f});
				draw_dashed_line(queue, bottom_right, bottom_left,  2.0f, Rgba{1.f,0.8f, 0.25f, 1.0f});
				draw_dashed_line(queue, bottom_left,  top_left,     2.0f, Rgba{1.f,0.8f, 0.25f, 1.0f});

				_batch.insert(*_icon_layer, top_left, {32.f, 32.f});
				_batch.insert(*_icon_rotate, bottom_right, {32.f, 32.f});
				_batch.insert(*_icon_scale, top_right, {32.f, 32.f});
				_batch.flush(queue);
			}
		}
	}
	auto Selection::update(util::maybe<glm::vec2> mp1, util::maybe<glm::vec2> mp2) -> bool {
		bool input_used = false;

		if(mp1.is_some() && true /* TODO: check if prev is inside entity */) {
			input_used = true;

			if(mp2.is_nothing()) {
				// single-touch
				util::process(_last_primary_pointer_pos, mp1) >> [&](auto prev, auto curr) {
					if(false /* TODO: check if this is a smart_texture */) {
						if(false /* TODO: check if prev is inside of a marker or near a border */) {
							// ...
							return;
						} else {
							this->_move(curr - prev);
						}

					} else if(false /* TODO: check if prev is inside of one of the icons */) {
						// ...
						return;
					}

					this->_move(curr - prev);
				};

			} else {
				// mutli-touch
				auto mp1_curr = mp1.get_or_throw();
				auto mp1_prev = _last_primary_pointer_pos.get_or_other(mp1_curr);

				auto mp2_curr = mp2.get_or_throw();
				auto mp2_prev = _last_secondary_pointer_pos.get_or_other(mp2_curr);

				auto mp1_delta = mp1_curr - mp1_prev;
				_move(mp1_delta);


				auto offset_curr = mp2_curr - mp1_curr;
				auto offset_prev = mp2_prev - mp1_prev;

				auto offset_curr_len = glm::length(offset_curr);
				auto offset_prev_len = glm::length(offset_prev);

				if(offset_prev_len>0.f && offset_curr_len>0.f) {
					_scale(offset_curr_len / offset_prev_len);

					auto angle_curr = Angle{glm::atan(offset_curr.y, offset_curr.x)};
					auto angle_prev = Angle{glm::atan(offset_prev.y, offset_prev.x)};
					_rotate(mp1_curr, angle_curr - angle_prev);
				}
			}
		}

		_last_primary_pointer_pos = mp1;
		_last_secondary_pointer_pos = mp2;

		return input_used;
	}

	auto Selection::copy_content()const -> std::string {
		// TODO
		return "[]";
	}

	void Selection::_change_selection(glm::vec2 point) {
		// TODO
	}

	void Selection::_move(glm::vec2 offset) {
		// TODO
	}
	void Selection::_move_layer(float offset) {
		// TODO
	}
	void Selection::_rotate(glm::vec2 pivot, Angle offset) {
		// TODO
	}
	void Selection::_scale(float factor) {
		// TODO
	}

}
}
}
