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


namespace lux {
namespace sys {
namespace physics {

	class Physics_system;

	class Physics_comp : public ecs::Component<Physics_comp> {
		public:
			static constexpr const char* name() {return "Physics";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Physics_comp(ecs::Entity& owner);



		private:
			friend class Physics_system;
			// TODO
	};

}
}
}
