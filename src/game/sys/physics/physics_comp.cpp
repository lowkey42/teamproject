#define GLM_SWIZZLE

#include "physics_comp.hpp"

#include "physics_system.hpp"
#include "transform_comp.hpp"

#include "polygon_separator.hpp"

#include "../graphic/sprite_comp.hpp"
#include "../graphic/terrain_comp.hpp"

#include <core/utils/sf2_glm.hpp>

#include <Box2D/Box2D.h>


namespace lux {
namespace sys {
namespace physics {

	namespace {
		constexpr auto def_foot_friction = 500.0f;
	}

	sf2_structDef(Body_definition,
	              active,
	              kinematic,
	              shape,
	              linear_damping,
	              angular_damping,
	              fixed_rotation,
	              bullet,
	              friction,
	              resitution,
	              density,
	              size,
	              sensor,
	              velocity,
	              keep_position_force)

	sf2_enumDef(Body_shape, polygon, humanoid, circle)

	namespace {
		using body_ptr = std::unique_ptr<b2Body, void(*)(b2Body*)>;

		// body, foot, sensor_bottom, sensor_left, sensor_right
		using ub_res = std::tuple<b2Fixture*, glm::vec2>;

		auto update_body(b2World& world, body_ptr& body, const Body_definition& def,
		                 ecs::Entity& owner, b2BodyType btype) -> ub_res {
			b2Fixture* fixture_foot = nullptr;

			auto& transform_comp = owner.get<Transform_comp>().get_or_throw();

			if(body) {
				auto fixture = body->GetFixtureList();
				while(fixture) {
					auto to_delete = fixture;
					fixture = fixture->GetNext();
					body->DestroyFixture(to_delete);
				}

				body->SetTransform(b2Vec2{transform_comp.position().x.value(),
				                          transform_comp.position().y.value()},
				                   def.fixed_rotation ? 0.f : transform_comp.rotation());

				body->SetFixedRotation(def.fixed_rotation);
				body->SetBullet(def.bullet);
				body->SetLinearDamping(def.linear_damping);
				body->SetAngularDamping(def.angular_damping);

			} else {
				b2BodyDef bd;
				bd.angle = def.fixed_rotation ? 0.f : transform_comp.rotation();
				bd.angularDamping = def.angular_damping;
				bd.bullet = def.bullet;
				bd.fixedRotation = def.fixed_rotation;
				bd.linearDamping = def.linear_damping;
				bd.position.x = transform_comp.position().x.value();
				bd.position.y = transform_comp.position().y.value();
				bd.userData = &owner;
				bd.type = btype;
				body = body_ptr{world.CreateBody(&bd), +[](b2Body*b){if(b) b->GetWorld()->DestroyBody(b);}};
			}

			b2FixtureDef main_fixture;
			main_fixture.density = def.density;
			main_fixture.friction = def.friction;
			main_fixture.restitution = def.resitution;

			auto size = def.size * transform_comp.scale();
			if(glm::length2(size)<0.01f) {
				size = owner.get<graphic::Sprite_comp>().process(size, [&](auto& s){
					return s.size() * transform_comp.scale();
				});
			}
			if(glm::length2(size)<0.01f) {
				size = owner.get<graphic::Anim_sprite_comp>().process(size, [&](auto& s){
					return s.size() * transform_comp.scale();
				});
			}

			if(glm::length2(size)>0.01f) {
				auto half_size = size / 2.f;

				switch(def.shape){
					case Body_shape::polygon: {
						b2PolygonShape shape;
						shape.SetAsBox(half_size.x, half_size.y);
						main_fixture.shape = &shape;
						if(def.sensor) {
							main_fixture.isSensor = true;
						}
						body->CreateFixture(&main_fixture);
						break;
					}

					case Body_shape::circle: {
						b2CircleShape shape;
						shape.m_radius = std::min(half_size.x, half_size.y);
						main_fixture.shape = &shape;
						if(def.sensor) {
							main_fixture.isSensor = true;
						}
						body->CreateFixture(&main_fixture);
						break;
					}

					case Body_shape::humanoid:
						constexpr float foot_h = 0.1f;
						b2PolygonShape shape;
						b2Vec2 body_shape[] {
							{-half_size.x, -half_size.y+foot_h},
							{ half_size.x, -half_size.y+foot_h},
							{ half_size.x,  half_size.y},
							{-half_size.x,  half_size.y},
						};
						shape.Set(body_shape, 4);
						main_fixture.shape = &shape;
						if(def.sensor) {
							main_fixture.isSensor = true;
						}
						body->CreateFixture(&main_fixture);

						b2Vec2 foot_shape[] {
							{ half_size.x-0.01f,         -half_size.y+foot_h},
							{ half_size.x-foot_h*1.5f,  -half_size.y},
							{-half_size.x+foot_h*1.5f,  -half_size.y},
							{-half_size.x+0.01f,         -half_size.y+foot_h},
						};
						shape.Set(foot_shape, 4);
						main_fixture.shape = &shape;
						main_fixture.friction = def_foot_friction;
						main_fixture.restitution = 0.01f;
						if(def.sensor) {
							main_fixture.isSensor = true;
						}
						fixture_foot = body->CreateFixture(&main_fixture);
						break;
				}

				return std::make_tuple(fixture_foot, size);
			}

			auto terrain_comp_mb = owner.get<graphic::Terrain_comp>();
			if(terrain_comp_mb.is_some()) {
				auto& terrain_comp = terrain_comp_mb.get_or_throw();

				INVARIANT(def.shape==Body_shape::polygon, "A terrain can only be of polygonal shape!");

				auto vertices = terrain_comp.smart_texture().vertices();
				for(auto i=0u; i<vertices.size(); i+=3) {
					b2PolygonShape shape;
					b2Vec2 body_shape[] {
						{vertices[i+0].x, vertices[i+0].y},
						{vertices[i+1].x, vertices[i+1].y},
						{vertices[i+2].x, vertices[i+2].y}
					};
					shape.Set(body_shape, 3);
					main_fixture.shape = &shape;
					body->CreateFixture(&main_fixture);
				}

				//create_polygons(terrain_comp.smart_texture().points(), *body, main_fixture);
				return std::make_tuple(nullptr, glm::vec2{1,1});
			}

			FAIL("No component to determine the size from!!!");
			return std::make_tuple(nullptr, glm::vec2{1,1});
		}
	}

	void Dynamic_body_comp::load(sf2::JsonDeserializer& state,
	                             asset::Asset_manager&) {
		state.read(_def);
		_dirty = true;
	}
	void Dynamic_body_comp::save(sf2::JsonSerializer& state)const {
		if(_body) {
			auto vel = _body->GetLinearVelocity();
			_def.velocity = glm::vec2{vel.x, vel.y};
		}
		state.write(_def);
	}
	Dynamic_body_comp::Dynamic_body_comp(ecs::Entity& owner) : Component(owner), _body(nullptr, +[](b2Body*b){}) {
	}
	void Dynamic_body_comp::_update_body(b2World& world) {
		auto org_aabb = _body ? util::just(calc_aabb()) : util::nothing();

		std::tie(_fixture_foot, _size) =
		        update_body(world, _body, _def, owner(), _def.kinematic ? b2_kinematicBody : b2_dynamicBody);
		_body->SetLinearVelocity({_def.velocity.x,_def.velocity.y});

		_last_body_position = glm::vec2{_body->GetPosition().x, _body->GetPosition().y};
		_initial_position = glm::vec2{_body->GetPosition().x, _body->GetPosition().y};

		_dirty = false;

		org_aabb.process([&](auto aabb){
			auto new_aabb = this->calc_aabb();
			if(new_aabb.w>aabb.w) {
				auto pos = _body->GetPosition();
				_body->SetTransform({pos.x, pos.y - (new_aabb.w-aabb.w)}, _body->GetAngle());
			}
		});
	}

	auto Dynamic_body_comp::calc_aabb()const -> glm::vec4 {
		b2AABB result;
		result.lowerBound = b2Vec2{0,0};
		result.upperBound = b2Vec2{0,0};

		b2Transform trans = _body->GetTransform();
		const b2Fixture* first = _body->GetFixtureList();

		for(auto fixture = first; fixture; fixture = fixture->GetNext()) {
			b2AABB aabb;
			fixture->GetShape()->ComputeAABB(&aabb, trans, 0);
			if(fixture==first)
				result = aabb;
			else
				result.Combine(aabb);
		}

		return {result.lowerBound.x, result.lowerBound.y, result.upperBound.x, result.upperBound.y};
	}

	void Dynamic_body_comp::apply_force(glm::vec2 f) {
		if(_body) {
			_body->ApplyForceToCenter(b2Vec2{f.x,f.y}, true);
		}
	}
	void Dynamic_body_comp::foot_friction(bool enable) {
		if(_def.shape==Body_shape::humanoid && _fixture_foot) { // TODO: only execute if required
			_fixture_foot->SetFriction(enable ? def_foot_friction : 0.f);
			b2ContactEdge* contactEdge = _body->GetContactList();
			while (contactEdge) {
				contactEdge->contact->ResetFriction();
				contactEdge = contactEdge->next;
			}
		}
	}
	bool Dynamic_body_comp::has_ground_contact()const {
		if(!_body) return true;
		return glm::abs(_body->GetLinearVelocity().y) < 2.f;
	}
	auto Dynamic_body_comp::velocity()const -> glm::vec2 {
		if(!_body) return {};
		return {_body->GetLinearVelocity().x, _body->GetLinearVelocity().y};
	}
	void Dynamic_body_comp::velocity(glm::vec2 v)const {
		if(_body) {
			_body->SetLinearVelocity(b2Vec2{v.x,v.y});
		}
	}
	auto Dynamic_body_comp::mass()const -> float {
		if(!_body) return 1.f;
		return _body->GetMass();
	}
	void Dynamic_body_comp::_update_ground_info(Physics_system& world) {
		using namespace glm;

		if(_def.shape==Body_shape::humanoid && _body) {
			auto pos = remove_units(owner().get<Transform_comp>().get_or_throw().position()).xy();
			INVARIANT(!std::isnan(pos.x) && !std::isnan(pos.y), "Position is nan");
			INVARIANT(!std::isnan(_size.x) && !std::isnan(_size.y), "Size is nan");

			auto ground = world.raycast(pos, vec2{0,-1}, 20.f, owner());
			auto ground_dist   = ground.process(999.f, [](auto& m) {return m.distance;});
			_ground_normal = ground.process(vec2{0,1}, [](auto& m) {return m.normal;});
			_grounded = ground_dist*ground_dist < glm::length2(_size/2.f);

			if(!_grounded) {
				// check left/right side, too

				auto ground_l = world.raycast(pos-vec2{_size.x/2.f,0}, vec2{0,-1}, 20.f, owner());
				auto ground_l_dist   = ground_l.process(999.f, [](auto& m) {return m.distance;});

				auto ground_r = world.raycast(pos+vec2{_size.x/2.f,0}, vec2{0,-1}, 20.f, owner());
				auto ground_r_dist   = ground_r.process(999.f, [](auto& m) {return m.distance;});

				if(ground_l_dist<ground_dist && ground_l_dist<=ground_r_dist) {
					_grounded = ground_l_dist*ground_l_dist < glm::length2(_size/2.f);
					_ground_normal = ground_l.get_or_throw().normal;

				} else if(ground_r_dist<ground_dist && ground_r_dist<=ground_l_dist) {
					_grounded = ground_r_dist*ground_r_dist < glm::length2(_size/2.f);
					_ground_normal = ground_r.get_or_throw().normal;
				}
			}
		}
	}

	void Static_body_comp::load(sf2::JsonDeserializer& state,
	                            asset::Asset_manager& asset_mgr) {
		state.read(_def);
		_dirty = true;
	}
	void Static_body_comp::save(sf2::JsonSerializer& state)const {
		state.write(_def);
	}
	Static_body_comp::Static_body_comp(ecs::Entity& owner) : Component(owner), _body(nullptr, +[](b2Body*b){}) {
	}
	void Static_body_comp::_update_body(b2World& world) {
		update_body(world, _body, _def, owner(), b2_staticBody);
		_dirty = false;
	}

}
}
}
