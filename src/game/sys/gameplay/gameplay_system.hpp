/** System managing all gameplay components **********************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "enlightened_comp.hpp"
#include "player_tag_comp.hpp"
#include "light_tag_comps.hpp"

#include "../physics/physics_system.hpp"
#include "finish_marker_comp.hpp"

#include <core/renderer/camera.hpp>
#include <core/renderer/texture_batch.hpp>
#include <core/renderer/sprite_animation.hpp>
#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>
#include <core/utils/random.hpp>

#include <functional>


namespace lux {
	namespace renderer {
		class Command_queue;
		class Camera;
	}

namespace sys {
	namespace cam {
		class Camera_system;
	}
	namespace controller {
		class Controller_system;
	}

namespace gameplay {

	struct Level_finished {
	};

	class Gameplay_system {
		public:
			Gameplay_system(Engine&, ecs::Entity_manager&, physics::Physics_system& physics_world,
			                cam::Camera_system& camera_sys,
			                controller::Controller_system& controller_sys,
			                std::function<void()> reload);

			void update_pre_physic(Time);
			void update_post_physic(Time);
			void draw_blood(renderer::Command_queue&, const renderer::Camera& camera);

			auto game_time()const {return _game_timer;}

		private:
			struct Blood_stain {
				glm::vec2 position;
				Light_color color;
				Angle rotation;
				float scale;

				Blood_stain() = default;
				Blood_stain(glm::vec2 p, Light_color c, Angle r, float s)
				    : position(p), color(c), rotation(r), scale(s) {}
			};

			Engine& _engine;
			util::Mailbox_collection _mailbox;
			Enlightened_comp::Pool& _enlightened;
			Player_tag_comp::Pool& _players;
			Lamp_comp::Pool& _lamps;
			Finish_marker_comp::Pool& _finish_marker;
			physics::Physics_system& _physics_world;
			cam::Camera_system& _camera_sys;
			controller::Controller_system& _controller_sys;
			std::function<void()> _reload;
			util::random_generator _rng;

			Time _light_timer{0};
			Time _light_effects_timer{0};

			Time _game_timer{0};

			std::vector<Blood_stain> _blood_stains;

			mutable renderer::Texture_batch _blood_batch;
			renderer::Texture_ptr _blood_stain_textures[light_color_num];

			bool _level_finished = false;
			bool _first_update_after_reset = true;


			void _update_light(Time);
			auto _is_reflective(glm::vec2 p, Enlightened_comp& light, ecs::Entity* hit) -> Light_op_res;
			auto _is_solid(Enlightened_comp& light, ecs::Entity* hit) -> Light_op_res;
			void _on_smashed(ecs::Entity& e);
			void _on_animation_event(const renderer::Animation_event& event);

			void _on_collision(sys::physics::Collision&);
			void _on_contact(sys::physics::Contact&);

			void _handle_light_pending(Time, Enlightened_comp&);
			void _handle_light_disabled(Time, Enlightened_comp&);
			void _handle_light_enabled(Time, Enlightened_comp&);

			void _enable_light(Enlightened_comp&);
			void _disable_light(Enlightened_comp&, bool final_impulse=true, bool boost=true);

			void _color_player(Enlightened_comp&, Light_color new_color);
			auto _split_player(Enlightened_comp& base, Position offset,
			                   Light_color new_color) -> Enlightened_comp&;
	};

}
}
}
