#define GLM_SWIZZLE

#include "physics_system.hpp"

#include "transform_comp.hpp"

#include <Box2D/Box2D.h>


namespace lux {
namespace sys {
namespace physics {

	using namespace unit_literals;

	namespace {
		constexpr auto gravity_x = 0.f;
		constexpr auto gravity_y = -30.f;

		constexpr auto time_step = 1.f / 60;
		constexpr auto velocity_iterations = 10;
		constexpr auto position_iterations = 6;

		constexpr auto max_depth_offset = 2.f;
	}

	Physics_system::Physics_system(Engine&, ecs::Entity_manager& ecs)
	    : _bodies_dynamic(ecs.list<Dynamic_body_comp>()),
	      _bodies_static(ecs.list<Static_body_comp>()),
	      _world(std::make_unique<b2World>(b2Vec2{gravity_x,gravity_y})) {

		_world->SetAutoClearForces(false);
	}
	Physics_system::~Physics_system() {}

	void Physics_system::update(Time dt) {
		_dt_acc += dt.value();

		auto steps = static_cast<int>(_dt_acc / time_step);
		_dt_acc -= steps*time_step;

		_get_positions();
		for(auto i=0; i<steps; ++i) {
			_world->Step(time_step, velocity_iterations, position_iterations);
		}
		_world->ClearForces();
		_set_positions(steps/(steps+1.f) + _dt_acc / time_step);// TODO: interpolation

	}

	void Physics_system::_get_positions() {
		for(auto& comp : _bodies_dynamic) {
			if(!comp._body) {
				this->update_body_shape(comp);
			}

			auto& transform = comp.owner().get<Transform_comp>().get_or_throw();
			auto pos = remove_units(transform.position());
			comp._body->SetTransform(b2Vec2{pos.x, pos.y}, transform.rotation().value());

			comp._body->SetActive(comp._active && std::abs(pos.z) <= max_depth_offset);
		}

		for(auto& comp : _bodies_static) {
			if(!comp._body) {
				this->update_body_shape(comp);
			}

			auto& transform = comp.owner().get<Transform_comp>().get_or_throw();
			auto pos = remove_units(transform.position());
			comp._body->SetTransform(b2Vec2{pos.x, pos.y}, transform.rotation().value());

			comp._body->SetActive(comp._active && std::abs(pos.z) <= max_depth_offset);
		}
	}
	void Physics_system::_set_positions(float alpha) {
		for(Dynamic_body_comp& comp : _bodies_dynamic) {
			auto& transform = comp.owner().get<Transform_comp>().get_or_throw();

			auto b2_pos = comp._body->GetPosition();
			auto pos = glm::vec2{b2_pos.x, b2_pos.y};
			pos = glm::mix(remove_units(transform.position()).xy(), pos, alpha);
			transform.position({pos.x*1_m, pos.y*1_m, transform.position().z});// TODO: interpolation
			transform.rotation(Angle{comp._body->GetAngle()});
		}
	}

	void Physics_system::update_body_shape(Dynamic_body_comp& comp) {
		comp._update_body(*_world);
	}

	void Physics_system::update_body_shape(Static_body_comp& comp) {
		comp._update_body(*_world);
	}

}
}
}
