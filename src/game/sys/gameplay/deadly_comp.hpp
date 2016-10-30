/** Kills enlighted entities on touch ****************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/component.hpp>


namespace lux {
namespace sys {
namespace gameplay {

	class Deadly_comp : public ecs::Component<Deadly_comp> {
		public:
			static constexpr auto name() {return "Deadly";}

			Deadly_comp() = default;
			Deadly_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}
	};

}
}
}
