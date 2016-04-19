#include "controller_system.hpp"

#include "../physics/physics_comp.hpp"
#include "../physics/transform_comp.hpp"

#include <core/input/events.hpp>


namespace lux {
namespace sys {
namespace controller {

	using namespace unit_literals;

	namespace {
		constexpr auto max_jump_time = 0.25_s;
	}

	Controller_system::Controller_system(Engine& engine, ecs::Entity_manager& ecs)
	    : _mailbox(engine.bus()),
	      _input_controllers(ecs.list<Input_controller_comp>()) {

		_mailbox.subscribe_to([&](input::Continuous_action& e) {
			switch(e.id) {
				case "move_left"_strid:
					_move_left += e.begin ? 1.f : -1.f;
					break;
				case "move_right"_strid:
					_move_right += e.begin ? 1.f : -1.f;
					break;

				case "jump"_strid:
					_jump = e.begin;
					break;

				case "transform"_strid:
					_transform_pending = e.begin;
					if(!e.begin) {
						_transform = true;
					}
					break;
			}
		});
		_mailbox.subscribe_to([&](input::Range_action& e) {
			switch(e.id) {
				case "move"_strid:
					_target_dir = e.abs;
					_move_dir = e.abs.x;
					_mouse_look = false;
					break;

				case "mouse_look"_strid:
					_mouse_look = true;
					_target_dir = e.abs;
					break;
			}
		});
	}

	bool Controller_system::_jump_allowed(Input_controller_comp& c, physics::Dynamic_body_comp& body)const {
		if(c._jump_timer>max_jump_time)
			return false;

		if(c._jump_timer<=0_s) {
			return body.has_ground_contact(); // TODO: check ground contact
		}

		return true;
	}

	void Controller_system::update(Time dt) {
		_mailbox.update_subscriptions();

		// TODO: AI

		auto effective_move = _move_dir;
		if(_move_left>0)
			effective_move-=1;
		if(_move_right>0)
			effective_move+=1;

		effective_move = glm::clamp(effective_move, -1.f, 1.f);

		for(Input_controller_comp& c : _input_controllers) {
			auto& body = c.owner().get<physics::Dynamic_body_comp>().get_or_throw();
//			auto& ctransform = c.owner().get<physics::Transform_comp>().get_or_throw();
//			auto cpos = remove_units(ctransform.position()).xy();
//			auto dir = _mouse_look ? _target_dir-cpos : _target_dir;
			// TODO: apply dir to sprite

			if(_transform_pending) {
				// TODO: set animation
				body.active(false); // disable player collision
				continue;
			}
			if(_transform) {
				// TODO: toggle transformation
			}
			body.active(true); // TODO: false for light form

			if(glm::abs(effective_move)>0.001f) {
				//if(body.has_ground_contact()) {
					body.foot_friction(false);
					// TODO: still sucks. Maybe good enough for air-control. Try this one next: http://qandasys.info/stopping-on-a-slope-in-box2d/
					//if(glm::abs(body.velocity().x) <= c._max_speed) {
						auto vel_diff = (effective_move*c._max_speed) - body.velocity().x;
						if(body.has_ground_contact())
							vel_diff /=2.f;
						auto force = vel_diff * body.mass() / dt.value();
						body.apply_force(glm::vec2{force, 0.f}); // TODO: calculate force based on target velocity
					//}
/*
				} else {
					auto x_vel = effective_move*c._max_speed*0.5f;

					x_vel = glm::mix(x_vel, body.velocity().x, glm::abs(x_vel)>0.01f ? 0.2f : 0.8f);

					body.velocity({x_vel, body.velocity().y});
				}
*/
			} else {
				body.foot_friction(true);
			}

			if(_jump && _jump_allowed(c, body)) {
				c._jump_timer+=dt;
				body.apply_force(glm::vec2{0.f, c._jump_force});

			} else {
				c._jump_timer = 0_s;
			}
		}

		_transform = false;
	}

}
}
}
