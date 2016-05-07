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

	enum class Camera_move_type {
		lazy, centered
	};

	class Camera_system {
		public:
			Camera_system(Engine&, ecs::Entity_manager&);

			void reset_position(Position p);
			void type(Camera_move_type t) {_type_changed=_type!=t; _type = t;}

			void start_slow_lerp(Time t);

			auto camera()const -> auto& {return _camera;}
			auto screen_to_world(glm::vec2 screen_pos) const noexcept -> glm::vec3;

			void update(Time);

		private:
			Camera_target_comp::Pool& _targets;
			renderer::Camera_sidescroller _camera;
			bool _first_target = true;
			Position _last_target;
			std::array<Position, 4> _target_history;
			int _target_history_curr = 0;
			bool _moving = false;

			bool _type_changed = false;
			Camera_move_type _type = Camera_move_type::lazy;

			Time _slow_lerp_time {};
			Position _slow_lerp_start {};
			Position _slow_lerp_target {};
			Time _slow_lerp_remainder {};

			auto _calc_target() -> Position;
			auto _smooth_target(Position p, Time dt) -> Position;
	};

}
}
}
