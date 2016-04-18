#include "physics_comp.hpp"

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
		constexpr auto def_foot_friction = 5.0f;
	}

	sf2_structDef(Body_definition,
	              shape,
	              linear_damping,
	              angular_damping,
	              fixed_rotation,
	              bullet,
	              friction,
	              resitution,
	              density,
	              size)

	sf2_enumDef(Body_shape, polygon, humanoid, circle)

	namespace {
		// body, foot, sensor_bottom, sensor_left, sensor_right
		using ub_res = std::tuple<b2Body*, b2Fixture*, b2Fixture*, b2Fixture*, b2Fixture*>;

		auto update_body(b2World& world, b2Body* body, const Body_definition& def,
		                 ecs::Entity& owner, b2BodyType btype) -> ub_res {
			b2Fixture* fixture_foot = nullptr;
			b2Fixture* fixture_sensor_botton = nullptr;
			b2Fixture* fixture_sensor_left = nullptr;
			b2Fixture* fixture_sensor_right = nullptr;

			if(body) {
				world.DestroyBody(body);
				body = nullptr;
			}

			auto& transform_comp = owner.get<Transform_comp>().get_or_throw();

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
			body = world.CreateBody(&bd);

			b2FixtureDef main_fixture;
			main_fixture.density = def.density;
			main_fixture.friction = def.friction;
			main_fixture.restitution = def.resitution;

			auto size = def.size;
			if(glm::length2(size)<0.01f) {
				size = owner.get<graphic::Sprite_comp>().process(size, [&](auto& s){
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
						body->CreateFixture(&main_fixture);
						break;
					}

					case Body_shape::circle: {
						b2CircleShape shape;
						shape.m_radius = std::min(half_size.x, half_size.y);
						main_fixture.shape = &shape;
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
						body->CreateFixture(&main_fixture);

						b2Vec2 foot_shape[] {
							{ half_size.x,         -half_size.y+foot_h},
							{ half_size.x-foot_h,  -half_size.y},
							{-half_size.x+foot_h,  -half_size.y},
							{-half_size.x,         -half_size.y+foot_h},
						};
						shape.Set(foot_shape, 4);
						main_fixture.shape = &shape;
						main_fixture.friction = def_foot_friction;
						main_fixture.restitution = 0.1f;
						fixture_foot = body->CreateFixture(&main_fixture);

						// TODO: sensors
						break;
				}

				return std::make_tuple(body, fixture_foot, fixture_sensor_botton,
				                       fixture_sensor_left, fixture_sensor_right);
			}

			auto terrain_comp_mb = owner.get<graphic::Terrain_comp>();
			if(terrain_comp_mb.is_some()) {
				auto& terrain_comp = terrain_comp_mb.get_or_throw();

				INVARIANT(def.shape==Body_shape::polygon, "A terrain can only be of polygonal shape!");

				create_polygons(terrain_comp.smart_texture().points(), *body, main_fixture);
				return std::make_tuple(body, nullptr, nullptr, nullptr, nullptr);
			}

			FAIL("No component to determine the size from!!!");
			return std::make_tuple(nullptr, nullptr, nullptr, nullptr, nullptr);
		}
	}

	void Dynamic_body_comp::load(sf2::JsonDeserializer& state,
	                             asset::Asset_manager&) {
		state.read(_def);
		_body = nullptr;
	}
	void Dynamic_body_comp::save(sf2::JsonSerializer& state)const {
		state.write(_def);
	}
	Dynamic_body_comp::Dynamic_body_comp(ecs::Entity& owner) : Component(owner) {
	}
	void Dynamic_body_comp::_update_body(b2World& world) {
		std::tie(_body, _fixture_foot, _fixture_sensor_botton,
		         _fixture_sensor_left, _fixture_sensor_right) =
		        update_body(world, _body, _def, owner(), b2_dynamicBody);
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


	void Static_body_comp::load(sf2::JsonDeserializer& state,
	                            asset::Asset_manager& asset_mgr) {
		state.read(_def);
		_body = nullptr;
	}
	void Static_body_comp::save(sf2::JsonSerializer& state)const {
		state.write(_def);
	}
	Static_body_comp::Static_body_comp(ecs::Entity& owner) : Component(owner) {
	}
	void Static_body_comp::_update_body(b2World& world) {
		std::tie(_body, std::ignore, std::ignore, std::ignore, std::ignore) =
		        update_body(world, _body, _def, owner(), b2_staticBody);
	}

}
}
}
