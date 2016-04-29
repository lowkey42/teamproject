#define GLM_SWIZZLE

#include "gameplay_system.hpp"

#include "collectable_comp.hpp"

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
		constexpr auto blood_stain_radius = 1.0f;

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
	      _reload(reload),
	      _blood_batch(64,true) {

		ecs.register_component_type<Collectable_comp>();

		_mailbox.subscribe_to([&](sys::physics::Collision& c){
			ecs::Entity* player = nullptr;
			if(c.a && c.a->has<Player_tag_comp>()) {
				player = c.a;
			} else if(c.b && c.b->has<Player_tag_comp>()) {
				player = c.b;
			}

			if(c.a && c.a->has<Collectable_comp>() && !c.a->get<Collectable_comp>().get_or_throw()._collected) {
				c.a->get<Collectable_comp>().get_or_throw()._collected = true;
				c.a->manager().erase(c.a->shared_from_this());
				return;
			}
			if(c.b && c.b->has<Collectable_comp>() && !c.b->get<Collectable_comp>().get_or_throw()._collected) {
				c.b->get<Collectable_comp>().get_or_throw()._collected = true;
				c.b->manager().erase(c.b->shared_from_this());
				return;
			}

			if(c.impact>=40.f && player) {
				DEBUG("Smashed to death: "<<c.a<<" <=>"<<c.b<<" | "<<c.impact);
				_on_smashed(*player);
			}
		});
	}

	void Gameplay_system::draw(renderer::Command_queue& q, const renderer::Camera&)const {
		// TODO: add magic to draw the blood stains
		_blood_batch.layer(0.09f);
		for(auto& bs : _blood_stains) {
			_blood_batch.insert(*_engine.assets().load<renderer::Texture>("tex:blood_stain.png"_aid), bs.position, glm::vec2{blood_stain_radius*2,blood_stain_radius*2});
		}

		_blood_batch.flush(q);

	}
	void Gameplay_system::update(Time dt) {
		_update_light(dt);
		_mailbox.update_subscriptions();

		if(_game_timer<=0_s) {
			if(_controller_sys.input_active())
				_game_timer+=dt;
		} else {
			_game_timer += dt;
		}

		if(_players.size()==0) {
			DEBUG("Everyone is dead!");

			// set camera to slowly lerp back to player and block input
			_camera_sys.start_slow_lerp(1.0_s);
			_controller_sys.block_input(0.8_s);

			_reload();
			_game_timer = 0_s;
		}
	}

	bool Gameplay_system::_is_reflective(glm::vec2 p) {
		for(auto& bs : _blood_stains) {
			if(glm::length2(bs.position-p)<blood_stain_radius*blood_stain_radius)
				return true;
		}
		return false;
	}
	void Gameplay_system::_on_smashed(ecs::Entity& e) {
		// TODO: play death animation
		// TODO: play sound

		_engine.audio_ctx().play_static(*_engine.assets().load<audio::Sound>("sound:slime"_aid));

		// TODO: when done => spawn Blood_stain, delete entity
		auto& transform = e.get<physics::Transform_comp>().get_or_throw();
		_blood_stains.emplace_back(remove_units(transform.position()).xy());

		e.manager().erase(e.shared_from_this());
	}

	void Gameplay_system::_update_light(Time dt) {
		bool any_one_lighted = false;
		bool any_one_not_grounded = false;

		for(Enlightened_comp& c : _enlightened) {
			auto& transform = c.owner().get<physics::Transform_comp>().get_or_throw();
			auto& body = c.owner().get<physics::Dynamic_body_comp>().get_or_throw();

			c._direction = glm::normalize(c._direction);

			if(c._state!=Enlightened_comp::State::enabled) {
				c._air_time = 0_s;
			}

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

						auto pos = remove_units(transform.position());
						auto ray = _physics_world.raycast(pos.xy(), c._direction, c._final_booster_distance, c.owner());
						if(ray.is_some() && ray.get_or_throw().distance <= c._final_booster_distance) {
							pos.x += c._direction.x * (ray.get_or_throw().distance-c._radius);
							pos.y += c._direction.y * (ray.get_or_throw().distance-c._radius);
							transform.position(pos * 1_m);
							_on_smashed(c.owner());
						} else {
							body.velocity(c._direction * c._velocity);
							body.apply_force(c._direction * c._velocity * 20.f);
						}

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

						if(_is_reflective(pos.xy()+direction*move_distance)) {
							auto normal = ray.get_or_throw().normal;
							c._direction = c._direction - 2.f*normal*dot(normal, c._direction);
							c._air_time = 0_s;

						} else {
							// TODO: set animation, stop effects, ...
							c._state = Enlightened_comp::State::disabled;
							c._was_light = false;
						}
					}

					pos.x += direction.x * move_distance;
					pos.y += direction.y * move_distance;
					transform.position(pos * 1_m);

					auto angle = Angle{glm::atan(-c._direction.x, c._direction.y)};
					transform.rotation(angle);

					if(c._max_air_time>0.f) {
						c._air_time+=dt;
						if(c._air_time >= c._max_air_time*1_s) {
							// TODO: set animation, stop effects, ...
							c._state = Enlightened_comp::State::disabled;
							c._was_light = false;
						}
					}

					break;
				}
			}

			body.active(c.disabled());
			transform.scale(c.disabled() ? 1.f : 0.5f); // TODO: debug onyl => delete

			any_one_lighted |= c.enabled();
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
