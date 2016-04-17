#include "physics_comp.hpp"

#include "transform_comp.hpp"
#include "polygon_separator.hpp"

#include "../graphic/sprite_comp.hpp"
#include "../graphic/terrain_comp.hpp"

#include <Box2D/Box2D.h>


namespace lux {
namespace sys {
namespace physics {

	sf2_structDef(Body_definition,
	              shape,
	              linear_damping,
	              angular_damping,
	              fixed_rotation,
	              bullet,
	              friction,
	              resitution,
	              density)

	sf2_enumDef(Body_shape, polygon, humanoid, circle)

	namespace {
		auto update_body(b2World& world, b2Body* body, const Body_definition& def,
		                 ecs::Entity& owner, b2BodyType btype) -> b2Body* {
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


			auto sprite_comp_mb = owner.get<graphic::Sprite_comp>();
			if(sprite_comp_mb.is_some()) {
				auto& sprite_comp = sprite_comp_mb.get_or_throw();
				auto size = sprite_comp.size() * transform_comp.scale();

				switch(def.shape){
					case Body_shape::polygon: {
						b2PolygonShape shape;
						shape.SetAsBox(size.x, size.y);
						main_fixture.shape = &shape;
						body->CreateFixture(&main_fixture);
						break;
					}

					case Body_shape::circle: {
						b2CircleShape shape;
						shape.m_radius = std::min(size.x, size.y) / 2.f;
						main_fixture.shape = &shape;
						body->CreateFixture(&main_fixture);
						break;
					}

					case Body_shape::humanoid:
						// TODO: design real player fixtures
						b2PolygonShape shape;
						shape.SetAsBox(size.x, size.y);
						main_fixture.shape = &shape;
						body->CreateFixture(&main_fixture);
						break;
				}

				return body;
			}

			auto terrain_comp_mb = owner.get<graphic::Terrain_comp>();
			if(terrain_comp_mb.is_some()) {
				auto& terrain_comp = terrain_comp_mb.get_or_throw();

				INVARIANT(def.shape==Body_shape::polygon, "A terrain can only be of polygonal shape!");

				create_polygons(terrain_comp.smart_texture().points(), *body, main_fixture);
				return body;
			}

			FAIL("No component to determine the size from!!!");
			return nullptr;
		}
	}

	void Dynamic_body_comp::load(sf2::JsonDeserializer& state,
	                             asset::Asset_manager&) {
		state.read(_def);
	}
	void Dynamic_body_comp::save(sf2::JsonSerializer& state)const {
		state.write(_def);
	}
	Dynamic_body_comp::Dynamic_body_comp(ecs::Entity& owner) : Component(owner) {
	}
	void Dynamic_body_comp::_update_body(b2World& world) {
		_body = update_body(world, _body, _def, owner(), b2_dynamicBody);
	}

	void Dynamic_body_comp::apply_force(glm::vec2 f) {
		_body->ApplyForceToCenter(b2Vec2{f.x,f.y}, true);
	}
	void Dynamic_body_comp::foot_friction(bool enable) {
		if(_def.shape==Body_shape::humanoid) {
			// TODO: set friction of foot fixture
		}
	}
	bool Dynamic_body_comp::has_ground_contact()const {
		return glm::abs(_body->GetLinearVelocity().y) < 2.f;
	}
	auto Dynamic_body_comp::velocity()const -> glm::vec2 {
		return {_body->GetLinearVelocity().x, _body->GetLinearVelocity().y};
	}


	void Static_body_comp::load(sf2::JsonDeserializer& state,
	                            asset::Asset_manager& asset_mgr) {
		state.read(_def);
	}
	void Static_body_comp::save(sf2::JsonSerializer& state)const {
		state.write(_def);
	}
	Static_body_comp::Static_body_comp(ecs::Entity& owner) : Component(owner) {
	}
	void Static_body_comp::_update_body(b2World& world) {
		_body = update_body(world, _body, _def, owner(), b2_staticBody);
	}

}
}
}
