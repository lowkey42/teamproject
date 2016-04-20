/** An entity that participates in collision resolution **********************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


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
		Body_shape shape = Body_shape::polygon;
		float linear_damping = 0.f;
		float angular_damping = 0.f;
		bool fixed_rotation = false;
		bool bullet = false;
		float friction = 1.f;
		float resitution = 0.3f;
		float density = 2.f;
		glm::vec2 size;
	};

	class Dynamic_body_comp : public ecs::Component<Dynamic_body_comp> {
		public:
			static constexpr const char* name() {return "Dynamic_body";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Dynamic_body_comp(ecs::Entity& owner);

			void apply_force(glm::vec2 f);
			void foot_friction(bool enable);//< only for humanoids
			bool has_ground_contact()const;
			void active(bool e) {_active = e;}

			auto velocity()const -> glm::vec2;
			void velocity(glm::vec2 v)const;

			auto mass()const -> float;
			auto size()const {return _size;}

		private:
			friend class Physics_system;

			Body_definition _def;
			b2Body* _body = nullptr;
			b2Fixture* _fixture_foot = nullptr;
			bool _active = true;
			glm::vec2 _size;

			void _update_body(b2World& world);
	};

	class Static_body_comp : public ecs::Component<Static_body_comp> {
		public:
			static constexpr const char* name() {return "Static_body";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Static_body_comp(ecs::Entity& owner);

			void active(bool e) {_active = e;}

		private:
			friend class Physics_system;

			Body_definition _def;
			b2Body* _body = nullptr;
			bool _active = true;

			void _update_body(b2World& world);
	};

}
}
}
