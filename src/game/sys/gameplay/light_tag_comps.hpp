/** Tags for different interactions with light *******************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>

#include <cstdint>


namespace lux {
namespace sys {
namespace gameplay {

	enum class Light_color : uint8_t {
		black = 0b0,
		red   = 0b100, green   = 0b010, blue   = 0b001, // primaries
		cyan  = 0b011, magenta = 0b101, yellow = 0b110, // secondaries
		white = 0b111,
	};
#ifdef sf2_enumDef
	sf2_enumDef(Light_color,
		black,
		red, green, blue,
		cyan, magenta, yellow,
		white
	)
#endif


	inline auto interactive_color(Light_color filter, Light_color pred)noexcept {
		auto ci = static_cast<uint8_t>(pred);
		auto ri = static_cast<uint8_t>(filter);
		return static_cast<Light_color>(ci & ri);
	}
	inline auto not_interactive_color(Light_color filter, Light_color pred)noexcept {
		auto ci = static_cast<uint8_t>(pred);
		auto ri = static_cast<uint8_t>(filter);
		return static_cast<Light_color>(ci & ~ri);
	}
	inline auto operator<(Light_color lhs, Light_color rhs)noexcept {
		return static_cast<uint8_t>(lhs) < static_cast<uint8_t>(rhs);
	}

	struct Light_op_res {
		Light_color interactive;
		Light_color passive;

		inline auto operator<(Light_op_res rhs)const noexcept {
			return std::tie(interactive, passive)
			        < std::tie(rhs.interactive, rhs.passive);
		}
		inline auto operator!()const noexcept {
			return Light_op_res {passive, interactive};
		}
	};


	class Reflective_comp : public ecs::Component<Reflective_comp> {
		public:
			static constexpr const char* name() {return "Reflective";}
			void load(sf2::JsonDeserializer& state, asset::Asset_manager&)override;
			void save(sf2::JsonSerializer& state)const override;
			Reflective_comp(ecs::Entity& owner) : Component(owner) {}

			auto color()const noexcept {return _color;}

		private:
			friend class Gameplay_system;

			Light_color _color;
	};

	class Paintable_comp : public ecs::Component<Paintable_comp> {
		public:
			static constexpr const char* name() {return "Paintable";}
			Paintable_comp(ecs::Entity& owner) : Component(owner) {}

		private:
			friend class Gameplay_system;
	};

	class Transparent_comp : public ecs::Component<Transparent_comp> {
		public:
			static constexpr const char* name() {return "Transparent";}
			void load(sf2::JsonDeserializer& state, asset::Asset_manager&)override;
			void save(sf2::JsonSerializer& state)const override;
			Transparent_comp(ecs::Entity& owner) : Component(owner) {}

			auto color()const noexcept {return _color;}

		private:
			friend class Gameplay_system;

			Light_color _color;
	};

	/// changes player color
	class Lamp_comp : public ecs::Component<Lamp_comp> {
		public:
			static constexpr const char* name() {return "Lamp";}
			void load(sf2::JsonDeserializer& state, asset::Asset_manager&)override;
			void save(sf2::JsonSerializer& state)const override;
			Lamp_comp(ecs::Entity& owner) : Component(owner) {}

			auto color()const noexcept {return _color;}

			auto resulting_color(Light_color c)const noexcept {
				auto ci = static_cast<uint8_t>(c);
				auto ri = static_cast<uint8_t>(_color);

				return static_cast<Light_color>(ci | ri);
			}

			auto in_range(Position p)const -> bool;

		private:
			friend class Gameplay_system;

			Light_color _color;
			Angle       _angle;
			Distance    _max_distance;
			Angle       _rotation;
			Position    _offset;
	};

}
}
}
