#define GLM_SWIZZLE

#include "gameplay_system.hpp"

#include "../cam/camera_system.hpp"

#include "../physics/transform_comp.hpp"
#include "../physics/physics_system.hpp"


namespace lux {
namespace sys {
namespace gameplay {

	using namespace glm;
	using namespace unit_literals;


	namespace {
		float dot(vec2 a, vec2 b) {
			return a.x*b.x + a.y*b.y;
		}
	}

	Gameplay_system::Gameplay_system(Engine& engine, ecs::Entity_manager& ecs,
	                                 physics::Physics_system& physics_world,
	                                 cam::Camera_system& camera_sys)
	    : _mailbox(engine.bus()),
	      _enlightened(ecs.list<Enlightened_comp>()),
	      _physics_world(physics_world),
	      _camera_sys(camera_sys) {
	}

	void Gameplay_system::update(Time dt) {

		bool any_one_lighted = false;
		bool any_one_not_grounded = false;

		for(Enlightened_comp& c : _enlightened) {
			auto& transform = c.owner().get<physics::Transform_comp>().get_or_throw();
			auto& body = c.owner().get<physics::Dynamic_body_comp>().get_or_throw();

			c._direction = glm::normalize(c._direction);

			switch(c._state) {
				case Enlightened_comp::State::disabled:
					if(body.grounded()) {
						c._air_transforms_left = c._air_transformations;
					}
					break;

				case Enlightened_comp::State::pending:
					if(!c._was_light) {
						auto angle = Angle{glm::atan(-c._direction.x, c._direction.y)};
						transform.rotation(angle);
					}
					// TODO: set animation, start effects, ...
					break;

				case Enlightened_comp::State::canceling:
					c._state = Enlightened_comp::State::disabled;
					// TODO: set animation, stop effects, ...
					break;

				case Enlightened_comp::State::activating:
					if(c._was_light) { // to physical
						// TODO: change animation/effect
						c._state = Enlightened_comp::State::disabled;
						c._was_light = false;
						body.velocity(c._direction * c._velocity);

					} else { // to light
						// TODO: change animation/effect
						c._state = Enlightened_comp::State::enabled;
						c._was_light = true;
						if(!body.grounded()) {
							c._air_transforms_left--;
						}
					}
					break;

				case Enlightened_comp::State::enabled: {
					auto pos = remove_units(transform.position());

					auto move_distance = c._velocity * dt.value();
					auto direction = c._direction;

					auto ray = _physics_world.raycast(pos.xy(), direction, c._radius*2.f+move_distance, c.owner());

					// TODO: check if hit object is solid for light
					if(ray.is_some() && ray.get_or_throw().distance-move_distance<=c._radius) {
						move_distance = ray.get_or_throw().distance - c._radius;

						auto normal = ray.get_or_throw().normal;
						c._direction = c._direction - 2.f*normal*dot(normal, c._direction);

						auto angle = Angle{glm::atan(-c._direction.x, c._direction.y)};
						transform.rotation(angle);
					}

					pos.x += direction.x * move_distance;
					pos.y += direction.y * move_distance;
					transform.position(pos * 1_m);
					break;
				}
			}

			body.active(c.disabled());
			transform.scale(c.disabled() ? 1.f : 0.5f); // TODO: debug onyl => delete

			any_one_lighted |= !c.disabled();
			any_one_not_grounded |= !body.grounded();
		}

		if(any_one_lighted) {
			_light_timer = 1_s;
		}

		if(_light_timer>0_s && !any_one_not_grounded) {
			_light_timer -= dt;
		}

		_camera_sys.type(_light_timer>0_s ? cam::Camera_move_type::centered
		                                  : cam::Camera_move_type::lazy);

	}

}
}
}
