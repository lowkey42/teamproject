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

			void reset_position(Position p);

			auto camera()const -> auto& {return _camera;}

			void update(Time);

		private:
			Camera_target_comp::Pool& _targets;
			renderer::Camera_sidescroller _camera;
			bool _first_target = true;
			Position _last_target;
			std::array<Position, 4> _target_history;
			int _target_history_curr;
			bool _moving = false;

			auto _calc_target() -> Position;
			auto _smooth_target(Position p, Time dt) -> Position;
	};

}
}
}
