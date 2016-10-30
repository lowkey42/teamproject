/** Tags for different interactions with light *******************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/component.hpp>

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
	constexpr auto light_color_num = 8;
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
	inline auto operator|(Light_color lhs, Light_color rhs)noexcept {
		return static_cast<Light_color>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
	}
	inline auto operator~(Light_color lhs)noexcept {
		return static_cast<Light_color>(~static_cast<uint8_t>(lhs));
	}
	inline auto contains(Light_color filter, Light_color pred)noexcept {
		auto ci = static_cast<uint8_t>(pred);
		auto ri = static_cast<uint8_t>(filter);
		return static_cast<Light_color>(ci & ri) == filter;
	}
	inline auto to_rgb(Light_color c) {
		auto ci = static_cast<uint8_t>(c);
		return Rgb{
				ci&0b100 ? 1:0,
				ci&0b010 ? 1:0,
				ci&0b001 ? 1:0
		};
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
			friend void load_component(ecs::Deserializer& state, Reflective_comp&);
			friend void save_component(ecs::Serializer& state, const Reflective_comp&);

			Reflective_comp() = default;
			Reflective_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

			auto color()const noexcept {return _color;}

		private:
			friend class Gameplay_system;

			Light_color _color;
	};

	class Paintable_comp : public ecs::Component<Paintable_comp> {
		public:
			static constexpr const char* name() {return "Paintable";}

			Paintable_comp() = default;
			Paintable_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

		private:
			friend class Gameplay_system;
	};

	class Paint_comp : public ecs::Component<Paint_comp> {
		public:
			static constexpr const char* name() {return "Paint";}
			friend void load_component(ecs::Deserializer& state, Paint_comp&);
			friend void save_component(ecs::Serializer& state, const Paint_comp&);

			Paint_comp() = default;
			Paint_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

			auto color()const noexcept {return _color;}

		private:
			friend class Gameplay_system;

			Light_color _color;
			float _radius;
	};

	class Light_leech_comp : public ecs::Component<Light_leech_comp> {
		public:
			static constexpr const char* name() {return "Light_leech";}

			Light_leech_comp() = default;
			Light_leech_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

		private:
			friend class Gameplay_system;
	};

	class Transparent_comp : public ecs::Component<Transparent_comp> {
		public:
			static constexpr const char* name() {return "Transparent";}
			friend void load_component(ecs::Deserializer& state, Transparent_comp&);
			friend void save_component(ecs::Serializer& state, const Transparent_comp&);

			Transparent_comp() = default;
			Transparent_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

			auto color()const noexcept {return _color;}

		private:
			friend class Gameplay_system;

			Light_color _color;
	};

	/// changes player color
	class Lamp_comp : public ecs::Component<Lamp_comp> {
		public:
			static constexpr const char* name() {return "Lamp";}
			friend void load_component(ecs::Deserializer& state, Lamp_comp&);
			friend void save_component(ecs::Serializer& state, const Lamp_comp&);

			Lamp_comp() = default;
			Lamp_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

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
			Angle       _angle {0};
			Distance    _max_distance;
			Angle       _rotation {0};
			Position    _offset;
			Time        _cooldown_left {};
	};

	class Prism_comp : public ecs::Component<Prism_comp> {
		public:
			static constexpr const char* name() {return "Prism";}
			friend void load_component(ecs::Deserializer& state, Prism_comp&);
			friend void save_component(ecs::Serializer& state, const Prism_comp&);

			Prism_comp() = default;
			Prism_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

		private:
			friend class Gameplay_system;

			Position _offset_red;
			Position _offset_green;
			Position _offset_blue;
	};


}
}
}
