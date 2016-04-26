#define GLM_SWIZZLE

#include "camera_system.hpp"

#include "../physics/transform_comp.hpp"
#include "../physics/physics_comp.hpp"

#include <core/renderer/graphics_ctx.hpp>

namespace lux {
namespace sys {
namespace cam {

	using namespace unit_literals;

	namespace {
		constexpr auto cam_distance = 5_m;
	}

	Camera_system::Camera_system(Engine& engine, ecs::Entity_manager& ecs)
	    : _targets(ecs.list<Camera_target_comp>()),
	      _camera(engine.graphics_ctx().viewport(), 80_deg, cam_distance, 100_m) {
	}
	void Camera_system::reset_position(Position p) {
		std::fill(std::begin(_target_history), std::end(_target_history), p);
		_last_target = p;
	}

	auto Camera_system::_calc_target() -> Position {
		if(_targets.size()==0) {
			return _last_target;

		} else if(_targets.size()>1) {
			// TODO: magic?
			// calculate the avg position of all targets => x,y
			// use the max distance to the avg position to calculate the camera distance => z
			return _last_target;

		} else {
			auto& cam = *_targets.begin();
			auto& transform = cam.owner().get<physics::Transform_comp>().get_or_throw();
			auto grounded = cam.owner().get<physics::Dynamic_body_comp>().process(true, [](auto& b){return b.grounded();});

			auto x = transform.position().x;
			auto y = transform.position().y;

			if(_type == Camera_move_type::lazy) {
				if(!grounded && glm::abs(remove_unit(_last_target.y-y))<6.f) {
					y = _last_target.y;
				}

				if(glm::abs(remove_unit(_last_target.x-x))<8.f && !_moving)
					x = _last_target.x;

				_moving = glm::abs(remove_unit(_last_target.x-x))>0.05f;
			}

			return Position{x, y, cam_distance};
		}
	}

	auto Camera_system::_smooth_target(Position curr, Time dt) -> Position {
		auto sum = Position{0_m,0_m,0_m};
		for(auto v : _target_history) {
			sum += v;
		}

		_target_history_curr = (_target_history_curr+1) % _target_history.size();
		_target_history[_target_history_curr] = curr;

		curr = glm::mix(remove_units(sum)/float(_target_history.size()), remove_units(curr), dt.value()*10.f)*1_m;
		return curr;
	}

	void Camera_system::update(Time dt) {
		auto target = _calc_target();

		if(_first_target || _type_changed) {
			reset_position(target);
			_first_target = false;
			_type_changed = false;
		}

		if(_type == Camera_move_type::lazy) {
			target.x = glm::mix(_last_target.x.value(), target.x.value(), std::min(dt.value()*5.f,1.f))*1_m;
			target.y = glm::mix(_last_target.y.value(), target.y.value(), std::min(dt.value()*10.f,1.f))*1_m;
			target = _smooth_target(target, dt);

		} else {
			target = glm::mix(remove_units(_last_target), remove_units(target), std::min(dt.value()*30.f,1.f)) * 1_m;
		}
		_camera.position(target);

		_last_target = target;
	}
	auto Camera_system::screen_to_world(glm::vec2 screen_pos) const noexcept -> glm::vec3 {
		auto exp_p = remove_units(_last_target);
		exp_p.z = 0.0;
		return _camera.screen_to_world(screen_pos, exp_p);
	}

}
}
}
