/** The transformation of an entity in the scene *****************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "../../../core/ecs/ecs.hpp"
#include "../../../core/units.hpp"

namespace lux {
namespace sys {
namespace physics {

	class Transform_system;

	struct Cell_key {
		int32_t x,y;

		bool operator<(const Cell_key& rhs)const noexcept {
			return std::tie(x,y) < std::tie(rhs.x,rhs.y);
		}
		bool operator==(const Cell_key& rhs)const noexcept {
			return std::tie(x,y) == std::tie(rhs.x,rhs.y);
		}
		bool operator!=(const Cell_key& rhs)const noexcept {
			return !(*this==rhs);
		}
	};

	class Transform_comp : public ecs::Component<Transform_comp> {
		public:
			static constexpr const char* name() {return "Transform";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Transform_comp(ecs::Entity& owner)noexcept
			  : Component(owner) {}

			auto position()const noexcept {return _position;}
			void position(Position pos)noexcept;
			void move(Position o)noexcept {position(position() + o);}

			auto scale()const noexcept {return _scale;}
			void scale(float s)noexcept {_scale = s;}

			auto rotation()const noexcept {return _rotation;}
			void rotation(Angle a)noexcept;

			struct Persisted_state;
			friend struct Persisted_state;
		private:
			friend class Transform_system;

			enum class State {
				clear, dirty, uninitialized
			};

			Position _position;
			float _scale = 1.f;
			Angle _rotation;
			bool _rotation_fixed = false;

			Cell_key _cell_idx;
			State _dirty = State::uninitialized;
	};

}
}
}

namespace std {
	template <> struct hash<lux::sys::physics::Cell_key> {
		auto operator()(lux::sys::physics::Cell_key key)const noexcept -> size_t {
			return static_cast<size_t>(key.x)*31 + key.y;
		}
	};
}
