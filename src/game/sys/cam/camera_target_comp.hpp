/** A simple camera that follows its entity **********************************
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
namespace cam {

	class Camera_system;

	class Camera_target_comp : public ecs::Component<Camera_target_comp> {
		public:
			static constexpr const char* name() {return "Camera_target";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Camera_target_comp(ecs::Entity& owner);



		private:
			friend class Camera_system;
			// TODO
	};

}
}
}
