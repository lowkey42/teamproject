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

#include "../../entity_events.hpp"

#include <core/renderer/sprite_batch.hpp>
#include <core/renderer/camera.hpp>
#include <core/utils/messagebus.hpp>

#include "light_comp.hpp"


namespace mo {
namespace sys {
namespace light {

	constexpr auto max_lights = 4;
	constexpr auto light_uniforms = 3+5*max_lights;
	constexpr auto light_uniforms_size = 3+1+3+(3+1+1+3+1)*max_lights;

	class Light_system {
		public:
			Light_system(util::Message_bus& bus,
			             ecs::Entity_manager& entity_manager,
			             Rgb sun_light = Rgb{0.5, 0.5, 0.5},
			             glm::vec3 sun_dir = {0.1, -0.8, 0.4},
			             Rgb ambient_light = Rgb{0.01, 0.01, 0.01});

			void draw(renderer::Command_queue&, const renderer::Camera& camera)const;
			void update(Time dt);

		private:
			util::Mailbox_collection _mailbox;
			Light_comp::Pool& _lights;
			Rgb _sun_light;
			glm::vec3 _sun_dir;
			Rgb _ambient_light;
	};

}
}
}
