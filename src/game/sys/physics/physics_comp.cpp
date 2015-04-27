#include "physics_comp.hpp"

#include <sf2/sf2.hpp>
#include "../../../core/ecs/serializer_impl.hpp"

namespace game {
namespace sys {
namespace physics{

	using namespace core;
	using namespace core::unit_literals;

	struct DFloatS {
		float x, y;
	};
	sf2_structDef(DFloatS,
		sf2_member(x),
		sf2_member(y)
	)

	struct Physics_comp::Persisted_state {
		float radius;
		float mass;
		float restitution;
		float friction;
		float max_active_velocity;
		float active_acceleration;

		bool solid;

		DFloatS velocity;
		DFloatS acceleration;

		Persisted_state(const Physics_comp& c) :
			radius(c._body_radius.value()),
			mass(c.mass().value()),
			restitution(c._restitution),
			friction(c._friction),
			max_active_velocity(glm::sqrt(c._max_active_velocity2.value()) / (1_km/hour).value()),
			active_acceleration(c._active_acceleration.value()),
			solid(c._solid),
			velocity(DFloatS{c._velocity.x.value(), c._velocity.y.value()}),
			acceleration(DFloatS{c._acceleration.x.value(), c._acceleration.y.value()}) {
		}
	};

	sf2_structDef(Physics_comp::Persisted_state,
		sf2_member(radius),
		sf2_member(mass),
		sf2_member(restitution),
		sf2_member(friction),
		sf2_member(max_active_velocity),
		sf2_member(active_acceleration),
		sf2_member(solid),
		sf2_member(velocity),
		sf2_member(acceleration)
	)

	void Physics_comp::load(ecs::Entity_state& state){
		auto s = state.read_to(Persisted_state{*this});

		_body_radius = Distance(s.radius);
		mass(Mass(s.mass));
		_restitution = s.restitution;
		_friction = s.friction;
		_max_active_velocity2 = (s.max_active_velocity * s.max_active_velocity) * (1_km/hour);
		_active_acceleration = core::Speed_per_time(s.active_acceleration),
		_solid = s.solid;
		_velocity = {Speed(s.velocity.x), Speed(s.velocity.y)};
		_acceleration = {Speed_per_time(s.acceleration.x), Speed_per_time(s.acceleration.y)};
		_active = true;
	}
	void Physics_comp::store(ecs::Entity_state& state){
		state.write_from(Persisted_state{*this});
	}



	Physics_comp::Physics_comp(core::ecs::Entity& owner, core::Distance body_radius,
				core::Mass mass, float restitution, float friction, bool solid) noexcept
		: Component(owner),
		  _body_radius(body_radius),
		  _inv_mass(1.f/mass),
		  _restitution(restitution),
		  _friction(friction),
		  _solid(solid),
		  _active(true) {}

	void Physics_comp::accelerate_active(glm::vec2 dir)noexcept {
		if(glm::length2(core::remove_units(_velocity)) > _max_active_velocity2.value())
			return;

		auto len = glm::length2(dir);
		if(len>1.f) {
			dir /= glm::sqrt(len);
		}

		accelerate(_active_acceleration * dir * 3); // *3 to cancel out friction
	}
	void Physics_comp::accelerate(core::Acceleration acc)noexcept {
		_acceleration+=acc;
		if(!is_zero(acc))
			_active = true;
	}
	void Physics_comp::impulse(core::Dir_force force)noexcept {
		_velocity+=_inv_mass*force*core::unit_literals::second;
		_active = true;
	}
	void Physics_comp::apply_force(core::Dir_force force)noexcept {
		accelerate(_inv_mass*force);
	}
	void Physics_comp::velocity(core::Velocity velocity)noexcept {
		_velocity = velocity;
		if(!is_zero(velocity))
			_active = true;
	}

}
}
}
