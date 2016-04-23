/**  Manages the lifetime and updates of other systems ***********************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "level.hpp"

#include "sys/cam/camera_system.hpp"
#include "sys/controller/controller_system.hpp"
#include "sys/graphic/graphic_system.hpp"
#include "sys/light/light_system.hpp"
#include "sys/physics/transform_system.hpp"
#include "sys/physics/physics_system.hpp"

#include <core/engine.hpp>
#include <core/ecs/ecs.hpp>
#include <core/renderer/camera.hpp>
#include <core/renderer/command_queue.hpp>
#include <core/renderer/skybox.hpp>
#include <core/renderer/texture.hpp>
#include <core/renderer/shader.hpp>


namespace lux {

	enum class Update : unsigned int {
		none       = 0b000,
		animations = 0b001,
		movements  = 0b010
	};
	using Update_mask = unsigned int;

	constexpr Update_mask operator|(Update lhs, Update rhs)noexcept {
		return static_cast<Update_mask>(lhs) | static_cast<Update_mask>(rhs);
	}
	constexpr Update_mask operator|(Update_mask lhs, Update rhs)noexcept {
		return lhs | static_cast<Update_mask>(rhs);
	}
	constexpr bool operator&(Update_mask lhs, Update rhs)noexcept {
		return lhs & static_cast<Update_mask>(rhs);
	}

	constexpr auto update_all = Update::animations | Update::movements;


	class Meta_system {
		public:
			Meta_system(Engine& engine);
			~Meta_system();

			auto load_level(const std::string& id) -> Level_data;

			void update(Time dt, Update_mask mask=update_all);
			void update(Time dt, Update update=Update::none);
			void draw(util::maybe<const renderer::Camera&> cam = util::nothing());

			ecs::Entity_manager entity_manager;
			sys::physics::Scene_graph scene_graph;
			sys::physics::Physics_system physics;
			sys::controller::Controller_system controller;
			sys::cam::Camera_system camera;
			sys::light::Light_system lights;
			sys::graphic::Graphic_system renderer;

		private:
			struct Post_renderer;

			Engine& _engine;

			renderer::Skybox _skybox;
			std::unique_ptr<Post_renderer> _post_renderer;

	};

}
