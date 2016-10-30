/** An entity that participates in collision resolution **********************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/component.hpp>


class b2Body;
class b2Fixture;
class b2World;
class b2RevoluteJoint;

namespace lux {
namespace sys {
namespace physics {

	class Physics_system;

	enum class Body_shape {
		polygon, //< vertices are based on the graphical representation
		humanoid,
		circle
	};

	struct Body_definition {
		bool active = true;
		bool kinematic=false;
		Body_shape shape = Body_shape::polygon;
		float linear_damping = 0.f;
		float angular_damping = 0.f;
		bool fixed_rotation = false;
		bool bullet = false;
		float friction = 1.f;
		float resitution = 0.3f;
		float density = 2.f;
		glm::vec2 size;
		bool sensor = false;
		glm::vec2 velocity{};
		float keep_position_force = 0.f;
		std::vector<glm::vec2> vertices;
	};

	class Dynamic_body_comp : public ecs::Component<Dynamic_body_comp, ecs::Compact_index_policy,
	                                                ecs::Pool_storage_policy<64, Dynamic_body_comp>> {
		public:
			static constexpr const char* name() {return "Dynamic_body";}
			friend void load_component(ecs::Deserializer& state, Dynamic_body_comp&);
			friend void save_component(ecs::Serializer& state, const Dynamic_body_comp&);

			Dynamic_body_comp() = default;
			Dynamic_body_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner);

			void apply_force(glm::vec2 f);
			void foot_friction(bool enable);//< only for humanoids
			bool has_ground_contact()const;
			void active(bool e) {_def.active = e;}
			void kinematic(bool e) {_dirty|=_def.kinematic!=e; _def.kinematic=e;}
			auto kinematic()const noexcept {return _def.kinematic;}

			auto velocity()const -> glm::vec2;
			void velocity(glm::vec2 v)const;

			auto mass()const -> float;
			auto size()const {return _size;}

			auto grounded()const {return _grounded;}
			auto ground_normal()const {return _ground_normal;}

			auto calc_aabb()const -> glm::vec4;

		private:
			friend class Physics_system;

			mutable Body_definition _def;
			std::unique_ptr<b2Body, void(*)(b2Body*)> _body {nullptr, +[](b2Body*){}};
			b2Fixture* _fixture_foot = nullptr;
			bool _dirty = true;
			glm::vec2 _size;
			bool _grounded = true;
			glm::vec2 _ground_normal{0,1};
			glm::vec2 _last_body_position;
			uint_fast32_t _transform_revision = 0;
			glm::vec2 _initial_position;

			void _update_body(b2World& world);
			void _update_ground_info(Physics_system&);
	};

	class Static_body_comp : public ecs::Component<Static_body_comp, ecs::Compact_index_policy,
	                                               ecs::Pool_storage_policy<128, Static_body_comp>> {
		public:
			static constexpr const char* name() {return "Static_body";}
			friend void load_component(ecs::Deserializer& state, Static_body_comp&);
			friend void save_component(ecs::Serializer& state, const Static_body_comp&);

			Static_body_comp() = default;
			Static_body_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner);

			void active(bool e) {_def.active = e;}

		private:
			friend class Physics_system;

			Body_definition _def;
			std::unique_ptr<b2Body, void(*)(b2Body*)> _body {nullptr, +[](b2Body*){}};
			bool _dirty = true;
			uint_fast32_t _transform_revision = 0;

			void _update_body(b2World& world);
	};

}
}
}
