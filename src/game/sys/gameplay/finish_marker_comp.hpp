/** Marks the end of a level *************************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


namespace lux {
namespace sys {
namespace gameplay {

	class Finish_marker_comp : public ecs::Component<Finish_marker_comp> {
		public:
			static constexpr const char* name() {return "Finish_marker";}

			Finish_marker_comp(ecs::Entity& owner) : Component(owner) {}
	};

}
}
}
