/** The transformation of an entity in the scene *****************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/ecs/component.hpp>
#include <core/units.hpp>


namespace lux {
namespace sys {
namespace physics {

	class Transform_system;

	class Transform_comp : public ecs::Component<Transform_comp, ecs::Compact_index_policy,
	                                             ecs::Pool_storage_policy<256, Transform_comp>> {
		public:
			static constexpr auto name() {return "Transform";}
			friend void load_component(ecs::Deserializer& state, Transform_comp&);
			friend void save_component(ecs::Serializer& state, const Transform_comp&);

			Transform_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)noexcept
			  : Component(manager, owner) {}

			auto position()const noexcept {return _position;}
			void position(Position pos)noexcept;
			void move(Position o)noexcept {position(position() + o);}

			auto scale()const noexcept {return _scale;}
			void scale(float s)noexcept {_scale = s;}

			auto rotation()const noexcept {return _rotation;}
			void rotation(Angle a)noexcept;

			auto flip_horizontal()const noexcept {return _flip_horizontal;}
			auto flip_vertical()const noexcept {return _flip_vertical;}
			void flip_horizontal(bool f)noexcept;
			void flip_vertical(bool f)noexcept;

			auto changed_since(uint_fast32_t expected)const noexcept {return _revision!=expected;}
			auto revision()const noexcept {return _revision;}

			auto resolve_relative(glm::vec3 offset)const -> glm::vec3;

			struct Persisted_state;
			friend struct Persisted_state;

		private:
			Position _position;
			float _scale = 1.f;
			Angle _rotation;
			bool _rotation_fixed = false;
			bool _flip_horizontal = false;
			bool _flip_vertical = false;
			uint_fast32_t _revision = 1;
	};

}
}
}

