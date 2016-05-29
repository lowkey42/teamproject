#define GLM_SWIZZLE

#include "gameplay_system.hpp"
#include "collectable_comp.hpp"
#include "deadly_comp.hpp"
#include "finish_marker_comp.hpp"

#include "../cam/camera_system.hpp"
#include "../controller/controller_system.hpp"
#include "../physics/transform_comp.hpp"
#include "../physics/physics_system.hpp"
#include "../light/light_comp.hpp"

#include <core/audio/music.hpp>
#include <core/audio/sound.hpp>
#include <core/audio/audio_ctx.hpp>

#include <core/ecs/serializer.hpp>


namespace lux {
namespace sys {
namespace gameplay {

	using namespace glm;
	using namespace unit_literals;
	using namespace renderer;


	namespace {
		constexpr auto blood_stain_radius = 2.0f;

		float dot(vec2 a, vec2 b) {
			return a.x*b.x + a.y*b.y;
		}

		auto round_player_pos(physics::Transform_comp& transform, float alpha) {
			auto pos = remove_units(transform.position());
			auto npos = pos;
			npos.x = std::round(pos.x*10.0f) / 10.f;
			npos.y = std::ceil(pos.y*10.0f) / 10.f;
			transform.position(glm::mix(pos, npos, alpha) * 1_m);
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
	      _lamps(ecs.list<Lamp_comp>()),
	      _physics_world(physics_world),
	      _camera_sys(camera_sys),
	      _controller_sys(controller_sys),
	      _reload(reload),
	      _rng(util::create_random_generator()),
	      _blood_batch(64,true) {

		_blood_stain_textures[static_cast<uint8_t>(Light_color::white)] = engine.assets().load<Texture>("tex:blood_stain_white"_aid);
		_blood_stain_textures[static_cast<uint8_t>(Light_color::red)] = engine.assets().load<Texture>("tex:blood_stain_red"_aid);
		_blood_stain_textures[static_cast<uint8_t>(Light_color::green)] = engine.assets().load<Texture>("tex:blood_stain_green"_aid);
		_blood_stain_textures[static_cast<uint8_t>(Light_color::blue)] = engine.assets().load<Texture>("tex:blood_stain_blue"_aid);
		_blood_stain_textures[static_cast<uint8_t>(Light_color::cyan)] = engine.assets().load<Texture>("tex:blood_stain_cyan"_aid);
		_blood_stain_textures[static_cast<uint8_t>(Light_color::magenta)] = engine.assets().load<Texture>("tex:blood_stain_magenta"_aid);
		_blood_stain_textures[static_cast<uint8_t>(Light_color::yellow)] = engine.assets().load<Texture>("tex:blood_stain_yellow"_aid);

		ecs.register_component_type<Collectable_comp>();
		ecs.register_component_type<Reflective_comp>();
		ecs.register_component_type<Paintable_comp>();
		ecs.register_component_type<Transparent_comp>();
		ecs.register_component_type<Lamp_comp>();
		ecs.register_component_type<Light_leech_comp>();
		ecs.register_component_type<Prism_comp>();
		ecs.register_component_type<Deadly_comp>();
		ecs.register_component_type<Finish_marker_comp>();

		_mailbox.subscribe_to([&](sys::physics::Collision& c) {
			this->_on_collision(c);
		});
		_mailbox.subscribe_to([&](sys::physics::Contact& c) {
			if(c.a && c.b && ((c.a->has<Player_tag_comp>() && c.b->has<Finish_marker_comp>())
			               || (c.b->has<Player_tag_comp>() && c.a->has<Finish_marker_comp>()))) {
				_level_finished = true;
				_mailbox.send<Level_finished>();

				auto player = c.a->has<Player_tag_comp>() ? c.a : c.b;
				player->get<Enlightened_comp>().process([&](auto& e) {
					this->_disable_light(e, false, false);
					player->erase<Enlightened_comp>();
				});

				// TODO: trigger player animation
			}
		});
	}

	void Gameplay_system::draw_blood(renderer::Command_queue& q, const renderer::Camera&) {
		for(auto& bs : _blood_stains) {
			_blood_batch.insert(*_blood_stain_textures[static_cast<uint8_t>(bs.color)],
			                    bs.position, glm::vec2{blood_stain_radius,blood_stain_radius}*2.f*bs.scale,
			                    bs.rotation);
		}

		_blood_batch.flush(q);

	}
	void Gameplay_system::update(Time dt) {
		_update_light(dt);
		_mailbox.update_subscriptions();

		if(_level_finished) {
			return; //< we are done here
		}

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

	namespace {
		auto build_lopr(Light_color filter, Light_color pred) {
			return Light_op_res {
				interactive_color(filter, pred),
				not_interactive_color(filter, pred)
			};
		}
	}

	auto Gameplay_system::_is_reflective(glm::vec2 pos, Enlightened_comp& light,
	                                     ecs::Entity* hit) -> Light_op_res {
		if(hit==nullptr)
			return Light_op_res{Light_color::black, Light_color::white};;

		auto ref_res = hit->get<Reflective_comp>().process(Light_op_res{Light_color::black, Light_color::black},
		                                                   [&](auto& r) {
			return build_lopr(r.color(), light._color);
		});

		auto paint_res = hit->get<Paintable_comp>().process(Light_op_res{Light_color::black, Light_color::black},
		                                                   [&](auto& p) {
			auto reflected = Light_color::black;

			for(auto& bs : _blood_stains) {
				if(glm::length2(bs.position-pos)<blood_stain_radius*blood_stain_radius) {
					reflected = reflected | interactive_color(bs.color, light._color);
				}
			}

			return Light_op_res{reflected, not_interactive_color(reflected, light._color)};
		});


		return std::max(ref_res, paint_res);
	}
	auto Gameplay_system::_is_solid(Enlightened_comp& light, ecs::Entity* hit) -> Light_op_res {
		if(hit!=nullptr) {
			auto transparent = hit->get<Transparent_comp>();
			if(transparent.is_some()) {
				return !build_lopr(transparent.get_or_throw().color(), light._color);
			}
		}

		return Light_op_res{light._color, Light_color::black};
	}

	void Gameplay_system::_update_light(Time dt) {
		bool any_one_lighted = false;
		bool any_one_not_grounded = false;

		for(Enlightened_comp& c : _enlightened) {
			auto& body = c.owner().get<physics::Dynamic_body_comp>().get_or_throw();

			c._direction = glm::normalize(c._direction);

			if(c._state!=Enlightened_State::enabled) {
				c._air_time = 0_s;
			}

			switch(c._state) {
				case Enlightened_State::canceling:
					if(c._was_light) {
						_enable_light(c); //< re-enable
					} else {
						_disable_light(c, false, false); //< re-disable
					}
					break;

				case Enlightened_State::activating:
					if(_physics_world.query_intersection(body, [](ecs::Entity& e){return e.has<Light_leech_comp>();}).is_some()) {
						_disable_light(c, false, false);
					} else {
						if(c._was_light) { // to physical
							_disable_light(c);

						} else { // to light
							_enable_light(c);
						}
					}
					break;


				case Enlightened_State::pending:
					if(_physics_world.query_intersection(body, [](ecs::Entity& e){return e.has<Light_leech_comp>();}).is_some()) {
						_disable_light(c, false, false);
					} else {
						_handle_light_pending(dt, c);
					}
					break;

				case Enlightened_State::disabled:
					_handle_light_disabled(dt, c);
					break;

				case Enlightened_State::enabled:
					_handle_light_enabled(dt, c);
					break;
			}

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

	void Gameplay_system::_on_collision(sys::physics::Collision& c) {
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

		if(c.a && c.b) {
			process(c.a->get<Enlightened_comp>(), c.b->get<Enlightened_comp>())
			        >> [&](Enlightened_comp& ae, Enlightened_comp& be) {
				if(!ae._smashed && !be._smashed && !ae.enabled() && !be.enabled()) {
					_color_player(ae, ae._color|be._color);
					c.b->manager().erase(c.b->shared_from_this());
					be._smashed = true;
					c.b = nullptr;
				}
			};
		}

		auto smashable = [&](ecs::Entity* e) {
			return e && e->get<Enlightened_comp>().process(false, [&](auto& elc) {
				ecs::Entity* other = (c.a==&elc.owner()) ? c.b : c.a;
				auto deadly = other && other->has<Deadly_comp>();
				return (elc._final_booster_left>0_s || c.impact>=elc._smash_force || deadly) && elc.smash();
			});
		};

		if(smashable(c.a)) {
			_on_smashed(*c.a);
		}
		if(smashable(c.b)) {
			_on_smashed(*c.b);
		}
	}
	void Gameplay_system::_on_smashed(ecs::Entity& e) {
		// TODO: play death animation
		// TODO: play sound
		_engine.audio_ctx().play_static(*_engine.assets().load<audio::Sound>("sound:slime"_aid));


		// TODO: when done => spawn Blood_stain, delete entity
		auto light = e.get<Enlightened_comp>();
		if(light.is_some()) {
			auto& transform = e.get<physics::Transform_comp>().get_or_throw();
			auto pos = remove_units(transform.position()).xy();
			auto color = light.get_or_throw()._color;
			_blood_stains.emplace_back(pos, color,
			                           Angle::from_degrees(util::random_real(_rng, 0.f, 360.f)),
			                           util::random_real(_rng, 0.8f, 1.0f));

		}

		e.manager().erase(e.shared_from_this());
	}

	void Gameplay_system::_handle_light_pending(Time dt, Enlightened_comp& c) {
		if(!c._was_light) {
			auto& body = c.owner().get<physics::Dynamic_body_comp>().get_or_throw();
			body.active(true);
			body.kinematic(true);
			auto& transform = c.owner().get<physics::Transform_comp>().get_or_throw();

			auto angle = Angle{glm::atan(-c._direction.x, c._direction.y)};
			transform.rotation(angle);
			round_player_pos(transform, std::min(dt/0.1_s, 1.f));

			c.owner().get<light::Light_comp>().process([&](auto& l){
				l.brightness_factor(2.f);
			});
		}
		// TODO: set animation, start effects, ...
	}
	void Gameplay_system::_handle_light_disabled(Time dt, Enlightened_comp& c) {
		auto& transform = c.owner().get<physics::Transform_comp>().get_or_throw();
		auto& body = c.owner().get<physics::Dynamic_body_comp>().get_or_throw();

		body.active(true);
		body.kinematic(false);

		if(body.grounded()) {
			c._air_transforms_left = c._air_transformations;
		}
		if(c._final_booster_left>0_s) {
			body.velocity(c._direction * c._velocity);
			c._final_booster_left -= dt;
		}

		auto res_color = c._color;
		for(auto& lamp : _lamps) {
			if(lamp.in_range(transform.position())) {
				res_color = lamp.resulting_color(res_color);
			}
		}
		if(res_color!=c._color) {
			_color_player(c, res_color);
		}
	}
	void Gameplay_system::_handle_light_enabled(Time dt, Enlightened_comp& c) {
		auto& transform = c.owner().get<physics::Transform_comp>().get_or_throw();
		auto& body = c.owner().get<physics::Dynamic_body_comp>().get_or_throw();

		body.active(false);
		body.kinematic(false);

		auto pos = remove_units(transform.position());

		auto move_distance = std::abs(c._velocity * dt.value());
		auto direction = c._direction;

		float max_ray_dist = c._radius*2.f+move_distance;
		if(max_ray_dist>0.01f) {
			auto ray = _physics_world.raycast(pos.xy(), direction, max_ray_dist, c.owner());

			if(ray.is_some() && ray.get_or_throw().distance-move_distance<=c._radius*1.5f) {
				if(auto prism = ray.get_or_throw().entity->get<Prism_comp>()) {
					auto prism_pos = ray.get_or_throw().entity->get<physics::Transform_comp>().get_or_throw().position();
					prism_pos.z = pos.z*1_m;

					// TODO: cleanup
					switch(c._color) {
						case Light_color::white:
							transform.position(prism_pos);
							_split_player(c, prism.get_or_throw()._offset_red, Light_color::red)._direction = glm::normalize(remove_units(prism.get_or_throw()._offset_red.xy()));
							_split_player(c, prism.get_or_throw()._offset_green, Light_color::green)._direction = glm::normalize(remove_units(prism.get_or_throw()._offset_green.xy()));
							_color_player(c, Light_color::blue);
							transform.position(prism_pos + prism.get_or_throw()._offset_blue);
							c._direction = glm::normalize(remove_units(prism.get_or_throw()._offset_blue.xy()));
							break;

						case Light_color::cyan:
							transform.position(prism_pos);
							_split_player(c, prism.get_or_throw()._offset_green, Light_color::green)._direction = glm::normalize(remove_units(prism.get_or_throw()._offset_green.xy()));
							_color_player(c, Light_color::blue);
							transform.position(prism_pos + prism.get_or_throw()._offset_blue);
							c._direction = glm::normalize(remove_units(prism.get_or_throw()._offset_blue.xy()));
							break;

						case Light_color::magenta:
							transform.position(prism_pos);
							_split_player(c, prism.get_or_throw()._offset_red, Light_color::red)._direction = glm::normalize(remove_units(prism.get_or_throw()._offset_red.xy()));
							_color_player(c, Light_color::blue);
							transform.position(prism_pos + prism.get_or_throw()._offset_blue);
							c._direction = glm::normalize(remove_units(prism.get_or_throw()._offset_blue.xy()));
							break;

						case Light_color::yellow:
							transform.position(prism_pos);
							_split_player(c, prism.get_or_throw()._offset_red, Light_color::red)._direction = glm::normalize(remove_units(prism.get_or_throw()._offset_red.xy()));
							_color_player(c, Light_color::green);
							transform.position(prism_pos + prism.get_or_throw()._offset_green);
							c._direction = glm::normalize(remove_units(prism.get_or_throw()._offset_green.xy()));
							break;

						default:
							break; // nothing to do
					}

				} else {
					auto solid = _is_solid(c, ray.get_or_throw().entity);
					if(solid.interactive!=Light_color::black) {
						if(solid.passive!=Light_color::black) {
							auto offset = Position{direction.x * move_distance, direction.y * move_distance, 0_m};
							_split_player(c, offset, solid.passive);
							_color_player(c, solid.interactive);
						}

						move_distance = ray.get_or_throw().distance - c._radius*2.0f;

						auto collision_point = pos.xy()+direction*move_distance;
						auto reflective = _is_reflective(collision_point, c, ray.get_or_throw().entity);

						if(reflective.interactive!=Light_color::black) {
							if(reflective.passive!=Light_color::black) {
								auto offset = Position{direction.x * move_distance, direction.y * move_distance, 0_m};
								auto& new_light = _split_player(c, offset, reflective.passive);
								_disable_light(new_light, true, false);

								_color_player(c, reflective.interactive);
							}

							auto normal = ray.get_or_throw().normal;
							c._direction = c._direction - 2.f*normal*dot(normal, c._direction);
							c._air_time = 0_s;

						} else {
							_disable_light(c, true, false);
						}
					}
				}
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
				_disable_light(c, true, false);
			}
		}
	}


	void Gameplay_system::_enable_light(Enlightened_comp& c) {
		auto& transform = c.owner().get<physics::Transform_comp>().get_or_throw();
		auto& body = c.owner().get<physics::Dynamic_body_comp>().get_or_throw();

		// TODO: change animation/effect
		c._state = Enlightened_State::enabled;
		c._was_light = true;
		round_player_pos(transform, 1.f);
		if(!body.grounded()) {
			c._air_transforms_left--;
		}
	}
	void Gameplay_system::_disable_light(Enlightened_comp& c, bool final_impulse, bool boost) {
		auto& body = c.owner().get<physics::Dynamic_body_comp>().get_or_throw();

		// TODO: change animation/effect
		c._state = Enlightened_State::disabled;
		c._was_light = false;

		if(final_impulse) {
			body.velocity(c._direction * c._velocity);
			if(boost) {
				c._final_booster_left = c._final_booster_time * 1_s;
			}
		}

		auto& transform = c.owner().get<physics::Transform_comp>().get_or_throw();
		transform.scale(c.disabled() ? 1.f : 0.5f); // TODO: debug only => delete
		transform.rotation(0_deg);

		c.owner().get<light::Light_comp>().process([&](auto& l){
			l.brightness_factor(1.f);
		});
	}


	void Gameplay_system::_color_player(Enlightened_comp& c, Light_color new_color) {
		constexpr auto ns = "blueprint"_strid;
		asset::AID blueprint;

		switch(new_color) {
			case Light_color::black:
				c.owner().manager().erase(c.owner_ptr());
				return;

			case Light_color::red:
				blueprint = asset::AID{ns, "player_red"};
				break;
			case Light_color::green:
				blueprint = asset::AID{ns, "player_green"};
				break;
			case Light_color::blue:
				blueprint = asset::AID{ns, "player_blue"};
				break;

			case Light_color::cyan:
				blueprint = asset::AID{ns, "player_cyan"};
				break;
			case Light_color::magenta:
				blueprint = asset::AID{ns, "player_magenta"};
				break;
			case Light_color::yellow:
				blueprint = asset::AID{ns, "player_yellow"};
				break;

			case Light_color::white:
				blueprint = asset::AID{ns, "player_white"};
				break;
		}

		DEBUG("Colored player in "<<blueprint.name());
		ecs::apply_blueprint(_engine.assets(), c.owner(), blueprint);
	}

	auto Gameplay_system::_split_player(Enlightened_comp& c, Position offset,
										Light_color new_color) -> Enlightened_comp& {
		// TODO[low]: poor-mans clone
		auto n = c.owner().manager().restore(c.owner().manager().backup(c.owner_ptr()));

		auto& nc = n->get<Enlightened_comp>().get_or_throw();
		_color_player(nc, new_color);

		auto& transform = n->get<physics::Transform_comp>().get_or_throw();
		transform.move(offset);

		return nc;
	}

}
}
}
