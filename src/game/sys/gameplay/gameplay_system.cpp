#define GLM_SWIZZLE

#include "gameplay_system.hpp"

#include "../cam/camera_system.hpp"
#include "../controller/controller_system.hpp"
#include "../physics/transform_comp.hpp"
#include "../physics/physics_system.hpp"

#include <core/audio/music.hpp>
#include <core/audio/sound.hpp>
#include <core/audio/audio_ctx.hpp>


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
	                                 cam::Camera_system& camera_sys,
	                                 controller::Controller_system& controller_sys,
	                                 std::function<void()> reload)
	    : _engine(engine),
	      _mailbox(engine.bus()),
	      _enlightened(ecs.list<Enlightened_comp>()),
	      _players(ecs.list<Player_tag_comp>()),
	      _physics_world(physics_world),
	      _camera_sys(camera_sys),
	      _controller_sys(controller_sys),
	      _reload(reload) {

		_mailbox.subscribe_to([&](sys::physics::Collision& c){
			ecs::Entity* player = nullptr;
			if(c.a && c.a->has<Player_tag_comp>()) {
				player = c.a;
			} else if(c.b && c.b->has<Player_tag_comp>()) {
				player = c.b;
			}

			if(c.impact>=40.f && player) {
				// TODO: play death animation
				// TODO: play sound

				DEBUG("Smashed to death: "<<c.a<<" <=>"<<c.b<<" | "<<c.impact);
				_engine.audio_ctx().play_static(*_engine.assets().load<audio::Sound>("sound:slime"_aid));

				// TODO: when done => spawn Blood_stain, delete entity
				auto& transform = player->get<physics::Transform_comp>().get_or_throw();
				_blood_stains.emplace_back(remove_units(transform.position()).xy());

				player->manager().erase(player->shared_from_this());
			}
		});
	}

	void Gameplay_system::draw(renderer::Command_queue&, const renderer::Camera& camera)const {
		// TODO: add magic to draw the blood stains

	}
	void Gameplay_system::update(Time dt) {
		_update_light(dt);
		_mailbox.update_subscriptions();

		if(_players.size()==0) {
			DEBUG("Everyone is dead!");

			// set camera to slowly lerp back to player and block input
			_camera_sys.start_slow_lerp(1.5_s);
			_controller_sys.block_input(1.0_s);

			_reload();
		}
	}

	void Gameplay_system::_update_light(Time dt) {
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
					c._state = c._was_light ? Enlightened_comp::State::enabled
					                        : Enlightened_comp::State::disabled;
					// TODO: set animation, stop effects, ...
					break;

				case Enlightened_comp::State::activating:
					if(c._was_light) { // to physical
						// TODO: change animation/effect
						c._state = Enlightened_comp::State::disabled;
						c._was_light = false;
						body.velocity(c._direction * c._velocity);
						body.apply_force(c._direction * c._velocity * 40.f + vec2{0,100});

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

					auto move_distance = std::abs(c._velocity * dt.value());
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
