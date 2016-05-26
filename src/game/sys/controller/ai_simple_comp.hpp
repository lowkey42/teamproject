/** Simple Goomba-Like AI ****************************************************
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
namespace controller {

	class Ai_simple_comp : public ecs::Component<Ai_simple_comp> {
		public:
			static constexpr const char* name() {return "Ai_simple";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Ai_simple_comp(ecs::Entity& owner) : Component(owner) {}

		private:
			friend class Controller_system;

			float _velocity = 1.f;
			bool _moving_left = false;
	};

}
}
}
