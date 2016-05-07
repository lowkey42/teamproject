/** manages and renders lights ***********************************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "light_comp.hpp"

#include "../../entity_events.hpp"

#include <core/renderer/sprite_batch.hpp>
#include <core/renderer/camera.hpp>
#include <core/renderer/command_queue.hpp>
#include <core/renderer/shader.hpp>
#include <core/utils/messagebus.hpp>

#include <gsl.h>


namespace lux {
namespace sys {
namespace light {

	constexpr auto max_lights = 8;
	constexpr auto light_uniforms = 3+6*max_lights;
	constexpr auto light_uniforms_size = 3+1+3+(3+3+1+1+3+1)*max_lights;

	struct Light_info;

	class Light_system {
		public:
			Light_system(util::Message_bus& bus,
			             ecs::Entity_manager& entity_manager,
			             asset::Asset_manager& asset_manager,
			             Rgb sun_light = Rgb{0.5, 0.5, 0.5},
			             glm::vec3 sun_dir = {0.1, -0.8, 0.4},
			             float ambient_brightness = 0.1f);

			void config(Rgb sun_light, glm::vec3 sun_dir, float ambient_brightness) {
				_sun_light = sun_light;
				_sun_dir = sun_dir;
				_ambient_brightness = ambient_brightness;
			}

			auto shadowcaster_batch() -> auto& {return _shadowcaster_batch;}
			void prepare_draw(renderer::Command_queue&, const renderer::Camera& camera);
			void update(Time dt);

		private:
			util::Mailbox_collection _mailbox;
			Light_comp::Pool& _lights;
			renderer::Command_queue  _shadowcaster_queue;
			renderer::Sprite_batch   _shadowcaster_batch;
			renderer::Framebuffer    _occlusion_map;
			renderer::Framebuffer    _shadow_map;
			renderer::Shader_program _shadowcaster_shader;
			renderer::Shader_program _shadowmap_shader;

			Rgb _sun_light;
			glm::vec3 _sun_dir;
			float _ambient_brightness;


			void _setup_uniforms(renderer::IUniform_map& uniforms, const renderer::Camera& camera,
			                     gsl::span<Light_info>);
			void _draw_occlusion_map(std::shared_ptr<renderer::IUniform_map> uniforms);
	};

}
}
}
