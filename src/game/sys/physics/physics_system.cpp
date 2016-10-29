#define GLM_SWIZZLE

#include "physics_system.hpp"

#include "transform_comp.hpp"

#include <Box2D/Box2D.h>
#include <glm/gtx/norm.hpp>

#include <unordered_set>


namespace lux {
	struct Contact_key {
		ecs::Entity_handle a;
		ecs::Entity_handle b;
		Contact_key() = default;
		Contact_key(ecs::Entity_handle a, ecs::Entity_handle b) : a(std::min(a,b)), b(std::max(a,b)){}

		auto operator<(const Contact_key& rhs)const noexcept {
			return std::tie(a,b) < std::tie(rhs.a, rhs.b);
		}
		auto operator==(const Contact_key& rhs)const noexcept {
			return std::tie(a,b) == std::tie(rhs.a, rhs.b);
		}
	};
}

namespace std {
	template<>
	struct hash<lux::Contact_key> {
		size_t operator()(const lux::Contact_key& x) const {
			return static_cast<size_t>(x.a.pack()) * 31 + static_cast<size_t>(x.b.pack());
		}
	};
}

namespace lux {
namespace sys {
namespace physics {

	using namespace unit_literals;

	namespace {
		constexpr auto gravity_x = 0.f;
		constexpr auto gravity_y = -50.f;

		constexpr auto time_step = 1.f / 60;
		constexpr auto velocity_iterations = 10;
		constexpr auto position_iterations = 6;

		constexpr auto max_depth_offset = 2.f;
	}

	struct Physics_system::Contact_listener : public b2ContactListener {

		util::Message_bus& bus;
		std::unordered_map<Contact_key, int> _contacts;

		Contact_listener(Engine& e) : bus(e.bus()) {}

		void BeginContact(b2Contact* contact) override {
			auto a = ecs::to_entity_handle(contact->GetFixtureA()->GetBody()->GetUserData());
			auto b = ecs::to_entity_handle(contact->GetFixtureB()->GetBody()->GetUserData());

			auto& count = _contacts[Contact_key{a,b}];
			if(count++ == 0) {
				bus.send<Contact>(a, b, true);
			}
		}
		void EndContact(b2Contact* contact) override {
			auto a = ecs::to_entity_handle(contact->GetFixtureA()->GetBody()->GetUserData());
			auto b = ecs::to_entity_handle(contact->GetFixtureB()->GetBody()->GetUserData());

			auto& count = _contacts[Contact_key{a,b}];
			if(count-- == 1) {
				bus.send<Contact>(a, b, false);
				_contacts.erase(Contact_key{a,b});
			}
		}

		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override {
			auto a = ecs::to_entity_handle(contact->GetFixtureA()->GetBody()->GetUserData());
			auto b = ecs::to_entity_handle(contact->GetFixtureB()->GetBody()->GetUserData());

			if(a && b) {
				auto impact = 0.f;
				for(auto i=0; i<impulse->count; ++i) {
					impact+=impulse->normalImpulses[i];
				}

				bus.send<Collision>(a, b, impact);
			}
		}
	};

	Physics_system::Physics_system(Engine& engine, ecs::Entity_manager& ecs)
	    : _bodies_dynamic(ecs.list<Dynamic_body_comp>()),
	      _bodies_static(ecs.list<Static_body_comp>()),
	      _listener(std::make_unique<Contact_listener>(engine)),
	      _world(std::make_unique<b2World>(b2Vec2{gravity_x,gravity_y})) {

		_world->SetContactListener(_listener.get());
		_world->SetAutoClearForces(false);
	}
	Physics_system::~Physics_system() {}

	void Physics_system::update(Time dt) {
		_dt_acc += dt.value();

		auto steps = static_cast<int>(_dt_acc / time_step);
		_dt_acc -= steps*time_step;

		_get_positions();

		if(steps>0) {
			for(auto i=0; i<steps; ++i) {
				_reset_smooth_state();

				_world->Step(time_step, velocity_iterations, position_iterations);
			}
		}

		_world->ClearForces();
		_smooth_positions(_dt_acc / time_step);
	}

	void Physics_system::_get_positions() {
		for(auto& comp : _bodies_dynamic) {
			if(!comp._body || comp._dirty) {
				this->update_body_shape(comp);
			}

			auto& transform = comp.owner().get<Transform_comp>().get_or_throw();
			auto pos = remove_units(transform.position());

			auto active = comp._def.active && std::abs(pos.z) <= max_depth_offset;

			if(transform.changed_since(comp._transform_revision) || comp._body->IsActive()!=active) {
				comp._transform_revision = transform.revision();

				auto rot = comp._def.fixed_rotation ? 0.f : transform.rotation().value();
				comp._body->SetTransform(b2Vec2{pos.x, pos.y}, rot);
				comp._body->SetActive(active);
				comp._initial_position = pos.xy();
			}

			if(comp._def.keep_position_force>0.f) {
				comp._body->ApplyForceToCenter(-1 * comp._body->GetMass() * _world->GetGravity(), true);
			}
		}

		for(auto& comp : _bodies_static) {
			if(!comp._body || comp._dirty) {
				this->update_body_shape(comp);
			}

			auto& transform = comp.owner().get<Transform_comp>().get_or_throw();
			if(transform.changed_since(comp._transform_revision)) {
				comp._transform_revision = transform.revision();

				auto& transform = comp.owner().get<Transform_comp>().get_or_throw();
				auto pos = remove_units(transform.position());
				comp._body->SetTransform(b2Vec2{pos.x, pos.y}, transform.rotation().value());
				comp._body->SetActive(comp._def.active && std::abs(pos.z) <= max_depth_offset);
			}
		}
	}
	void Physics_system::_reset_smooth_state() {
		for(auto& comp : _bodies_dynamic) {
			auto b2_pos = comp._body->GetPosition();
			comp._last_body_position = glm::vec2{b2_pos.x, b2_pos.y};

			if(comp._def.keep_position_force>0.f) {
				auto diff = comp._last_body_position - comp._initial_position;
				auto diff_len = glm::length(diff);
				if(diff_len>0.2f) {
					diff/=diff_len;
					auto resp = -diff * comp._def.keep_position_force * glm::clamp(diff_len/10.f, 0.1f, 1.0f);
					comp._body->ApplyLinearImpulse(b2Vec2{resp.x, resp.y}, comp._body->GetWorldCenter(), true);
				}
			}
		}
	}

	void Physics_system::_smooth_positions(float alpha) {
		for(Dynamic_body_comp& comp : _bodies_dynamic) {
			auto& transform = comp.owner().get<Transform_comp>().get_or_throw();

			auto b2_pos = comp._body->GetPosition();
			auto pos = glm::vec2{b2_pos.x, b2_pos.y};
			pos = glm::mix(comp._last_body_position, pos, alpha);
			transform.position({pos.x*1_m, pos.y*1_m, transform.position().z});
			if(!comp._def.fixed_rotation) {
				transform.rotation(Angle{comp._body->GetAngle()});
			}

			comp._transform_revision = transform.revision();
			comp._update_ground_info(*this);
		}
	}

	void Physics_system::update_body_shape(Dynamic_body_comp& comp) {
		comp._update_body(*_world);
	}

	void Physics_system::update_body_shape(Static_body_comp& comp) {
		comp._update_body(*_world);
	}

	namespace {
		struct Simple_ray_callback : public b2RayCastCallback {
			float max_dist = 1.f;
			ecs::Entity_handle exclude;
			util::maybe<Raycast_result> result = util::nothing();

			Simple_ray_callback(float max_dist, ecs::Entity_handle exclude={})
			    : max_dist(max_dist), exclude(exclude) {
			}

			float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
			                      const b2Vec2& normal, float32 fraction) override {
				//if(fixture->IsSensor())
				//	return -1.f; // ignore

				auto hit_entity = ecs::to_entity_handle(fixture->GetBody()->GetUserData());
				if(exclude && hit_entity==exclude)
					return -1.f; // ignore

				result = Raycast_result{glm::vec2{normal.x, normal.y}, fraction*max_dist, hit_entity};
				return fraction;
			}

			bool ShouldQueryParticleSystem(const b2ParticleSystem*) override {
				return false;
			}
		};
	}

	auto Physics_system::raycast(glm::vec2 position, glm::vec2 dir,
	                             float max_dist) -> util::maybe<Raycast_result> {
		if(!(glm::length2(dir*max_dist)>0.f)) {
			WARN("raycast with 0 length");
			return util::nothing();
		}

		const auto target = position + dir*max_dist;

		Simple_ray_callback callback{max_dist};
		_world->RayCast(&callback, b2Vec2{position.x, position.y}, b2Vec2{target.x, target.y});

		return callback.result;
	}
	auto Physics_system::raycast(glm::vec2 position, glm::vec2 dir, float max_dist,
	                             ecs::Entity_handle exclude) -> util::maybe<Raycast_result> {
		if(!(glm::length2(dir*max_dist)>0.f)) {
			WARN("raycast with 0 length");
			return util::nothing();
		}
		const auto target = position + dir*max_dist;

		Simple_ray_callback callback{max_dist, exclude};
		_world->RayCast(&callback, b2Vec2{position.x, position.y}, b2Vec2{target.x, target.y});

		return callback.result;
	}

	namespace {
		struct Check_overlap_callback : b2QueryCallback{
			const b2Body& body;
			std::function<bool(ecs::Entity_handle)> filter;
			util::maybe<ecs::Entity_handle> result;

			Check_overlap_callback(const b2Body& body, std::function<bool(ecs::Entity_handle)> filter)
			    : body(body), filter(filter) {}

			bool ReportFixture(b2Fixture* fixture) override {
				if( fixture->GetBody() == &body )
					return true;

				for(auto bf = body.GetFixtureList(); bf; bf = bf->GetNext()) {
					auto entity = ecs::to_entity_handle(fixture->GetBody()->GetUserData());
					if(entity && b2TestOverlap(fixture->GetShape(), 0, bf->GetShape(), 0, fixture->GetBody()->GetTransform(), body.GetTransform()) && filter(entity)) {
						result = util::just(entity);
						return false;
					}
				}

				return true;
			}
		};
		auto calc_aabb(const b2Body& body) {
			b2AABB result;
			result.lowerBound = b2Vec2{0,0};
			result.upperBound = b2Vec2{0,0};

			b2Transform trans = body.GetTransform();
			const b2Fixture* first = body.GetFixtureList();

			for(auto fixture = first; fixture; fixture = fixture->GetNext()) {
				b2AABB aabb;
				fixture->GetShape()->ComputeAABB(&aabb, trans, 0);
				if(fixture==first)
					result = aabb;
				else
					result.Combine(aabb);
			}

			return result;
		}
	}
	auto Physics_system::query_intersection(Dynamic_body_comp& body,
	                                        std::function<bool(ecs::Entity_handle)> filter) -> util::maybe<ecs::Entity_handle> {
		if(!_world || !body._body)
			return util::nothing();

		Check_overlap_callback callback{*body._body, filter};
		_world->QueryAABB(&callback, calc_aabb(*body._body));
		return callback.result;
	}

}
}
}
