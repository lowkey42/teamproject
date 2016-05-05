/** Tag component marking player entities ************************************
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

	class Player_tag_comp : public ecs::Component<Player_tag_comp> {
		public:
			static constexpr const char* name() {return "Player_tag";}

			Player_tag_comp(ecs::Entity& owner) : Component(owner) {}
	};

}
}
}
