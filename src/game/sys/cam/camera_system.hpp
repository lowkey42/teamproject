/** System providing the current camera of the scene *************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "camera_target_comp.hpp"

#include <core/renderer/camera.hpp>
#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


namespace lux {
namespace sys {
namespace cam {

	class Camera_system {
		public:
			Camera_system(Engine&, ecs::Entity_manager&);

			auto camera()const -> auto& {return _camera;}

			void update(Time);

		private:
			Camera_target_comp::Pool& _targets;
			renderer::Camera_sidescroller _camera;
			Position _last_target;

			auto _calc_target() -> Position;
			// TODO
	};

}
}
}
