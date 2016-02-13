/**************************************************************************\
 * Manages other systems                                                  *
 *                                               ___                      *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2014 Florian Oetke                                       *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include "sys/graphic/graphic_system.hpp"
#include "sys/light/light_system.hpp"
#include "sys/physics/transform_system.hpp"

#include <core/engine.hpp>
#include <core/ecs/ecs.hpp>
#include <core/renderer/camera.hpp>
#include <core/renderer/command_queue.hpp>


namespace mo {

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

			void update(Time dt, Update_mask mask=update_all);
			void update(Time dt, Update update=Update::none);
			void draw(const renderer::Camera&)const;

			ecs::Entity_manager entity_manager;
			sys::physics::Scene_graph scene_graph;
			sys::light::Light_system lights;
			sys::graphic::Graphic_system renderer;

		private:
			mutable renderer::Command_queue _render_queue;
	};

}
