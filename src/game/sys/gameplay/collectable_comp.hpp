/** An entity that can be picked up by the player ****************************
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

	class Collectable_comp : public ecs::Component<Collectable_comp> {
		public:
			static constexpr auto name() {return "Collectable";}

			Collectable_comp() = default;
			Collectable_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

		private:
			friend class Gameplay_system;

			bool _collected = false;
	};

}
}
}
