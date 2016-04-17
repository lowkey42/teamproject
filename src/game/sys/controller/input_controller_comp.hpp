/** An entity that is directly controlled by an HID **************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/renderer/camera.hpp>
#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


namespace lux {
namespace sys {
namespace controller {

	class Controller_system;

	class Input_controller_comp : public ecs::Component<Input_controller_comp> {
		public:
			static constexpr const char* name() {return "Input_controller";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Input_controller_comp(ecs::Entity& owner);



		private:
			friend class Controller_system;

			float _move_force = 1.f;
			float _jump_force = 1.f;

			float _max_speed = 10.f;

			Time _jump_timer{0};
	};

}
}
}
