/** Marks the end of a level *************************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "light_tag_comps.hpp"

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/component.hpp>


namespace lux {
namespace sys {
namespace gameplay {

	class Finish_marker_comp : public ecs::Component<Finish_marker_comp> {
		public:
			static constexpr const char* name() {return "Finish_marker";}
			friend void load_component(ecs::Deserializer& state, Finish_marker_comp&);
			friend void save_component(ecs::Serializer& state, const Finish_marker_comp&);

			Finish_marker_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

			auto colors_left()const noexcept {
				return not_interactive_color(_contained_colors,_required_color);
			}
			void add_color(Light_color c)noexcept {
				_contained_colors = _contained_colors | c;
				_activated = true;
			}

		private:
			friend class Gameplay_system;

			Light_color _required_color = Light_color::black;
			Light_color _contained_colors = Light_color::black;
			bool _activated = false;
	};

}
}
}
