#define GLM_SWIZZLE

#include "camera_system.hpp"

#include "../physics/transform_comp.hpp"

#include <core/renderer/graphics_ctx.hpp>

namespace lux {
namespace sys {
namespace cam {

	using namespace unit_literals;

	constexpr auto cam_distance = 5_m;

	Camera_system::Camera_system(Engine& engine, ecs::Entity_manager& ecs)
	    : _targets(ecs.list<Camera_target_comp>()),
	      _camera(engine.graphics_ctx().viewport(), 80_deg, cam_distance, 100_m) {
	}

	auto Camera_system::_calc_target() -> Position {
		if(_targets.size()==0) {
			return _last_target;

		} else if(_targets.size()>1) {
			// TODO: magic?
			return _last_target;

		} else {
			auto& cam = *_targets.begin();
			return Position{cam.owner().get<physics::Transform_comp>().get_or_throw().position().xy(), cam_distance};
		}
	}

	void Camera_system::update(Time) {
		auto target = _calc_target();

		// TODO: should implement a more lazy camera. big dead-zone in the center, lerp to recenter,
		//       when dead-zone is left or recenter-y after a ground-contact has been reestablisht + always recenter when in light-form
		//       Maybe like the Super Mario World (platform-locked / water) camera.
		_camera.position(target);

		_last_target = target;
	}

}
}
}
