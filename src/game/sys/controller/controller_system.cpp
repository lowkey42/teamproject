#define GLM_SWIZZLE

#include "controller_system.hpp"

#include "../physics/physics_comp.hpp"
#include "../physics/transform_comp.hpp"
#include "../physics/physics_system.hpp"

#include <core/input/events.hpp>


namespace lux {
namespace sys {
namespace controller {

	using namespace unit_literals;
	using namespace glm;

	namespace {
		constexpr auto max_jump_time = 0.25_s;
	}

	Controller_system::Controller_system(Engine& engine, ecs::Entity_manager& ecs,
	                                     physics::Physics_system& physics_world)
	    : _mailbox(engine.bus()),
	      _input_controllers(ecs.list<Input_controller_comp>()),
	      _physics_world(physics_world) {

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

	bool Controller_system::_jump_allowed(Input_controller_comp& c, float ground_dist)const {
		if(c._jump_timer>max_jump_time)
			return false;

		if(c._jump_timer<=0_s) {
			return ground_dist<1.f;
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
			auto& ctransform = c.owner().get<physics::Transform_comp>().get_or_throw();
			auto cpos = remove_units(ctransform.position()).xy();
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


			auto ground =_physics_world.raycast(cpos, vec2{0,-1}, 20.f, c.owner());
			auto ground_dist   = ground.process(999.f, [](auto& m) {return m.distance;});
			auto ground_normal = ground.process(vec2{0,1}, [](auto& m) {return m.normal;});
			auto move_force = vec2{0,0};

			// TODO: player can still get stuck on some edges

			if(ground_dist>1.0f) { // we are in the air
				ctransform.rotation(Angle{glm::mix(ctransform.rotation().value(), 0.f, 20*dt.value())});

				// TODO: make air-velocity configurable
				auto vel_target = effective_move*c._max_speed * 0.5f;
				auto vel_diff = vel_target - body.velocity().x;
				move_force.x = vel_diff * body.mass() / dt.value();
				// TODO: allows running up ~80_deg walls, maybe use body.velocity().y<-1.f to limit the max-force on the way up

			} else { // we are on the ground
				auto ground_angle = Angle{glm::atan(-ground_normal.x, ground_normal.y)};

				ctransform.rotation(Angle{glm::mix(ctransform.rotation().value(), ground_angle.value()*0.75f, 10*dt.value())});

				if(abs(ground_angle)<40_deg) {
					// TODO: nicer acceleration curve (fast linear acceleration to walk-velocity,
					//			after N seconds acceleration to run-velocity)
					auto vel_target = effective_move*c._max_speed * 0.5f;
					auto vel_diff = vel_target - body.velocity().x;
					move_force.x = vel_diff * body.mass() / dt.value();
				} else {
					ground_dist = 999.f;
				}
			}

			body.foot_friction(glm::length2(move_force)<0.001f);

			body.apply_force(move_force);


			if(_jump && _jump_allowed(c, ground_dist)) {
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
