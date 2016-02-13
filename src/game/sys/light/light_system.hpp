/**************************************************************************\
 * manages and renders lights                                             *
 *                                                ___                     *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2015 Florian Oetke & Sebastian Schalow                   *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include "../physics/transform_system.hpp"

#include "../../entity_events.hpp"

#include <core/renderer/sprite_batch.hpp>
#include <core/renderer/camera.hpp>

#include "light_comp.hpp"


namespace mo {
namespace sys {
namespace light {

	constexpr auto max_lights = 6;
	constexpr auto light_uniforms = 3+5*max_lights;
	constexpr auto light_uniforms_size = 3+1+3+(3+1+1+3+1)*max_lights;

	class Light_system {
		public:
			Light_system(util::Message_bus& bus,
			             ecs::Entity_manager& entity_manager,
			             physics::Scene_graph& scene_graph,
			             Rgb sun_light = Rgb{0.5, 0.5, 0.5},
			             Angle sun_dir = Angle::from_degrees(90),
			             Rgb ambient_light = Rgb{0.5, 0.5, 0.5});

			void draw(renderer::Command_queue&, const renderer::Camera& camera)const;
			void update(Time dt);

		private:
			util::Mailbox_collection _mailbox;
			physics::Scene_graph& _scene_graph;
			Light_comp::Pool& _lights;
			Rgb _sun_light;
			Angle _sun_dir;
			Rgb _ambient_light;
	};

}
}
}
