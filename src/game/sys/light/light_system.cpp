#ifndef ANDROID
	#include <GL/glew.h>
	#include <GL/gl.h>
#else
	#include <GLES2/gl2.h>
#endif

#define GLM_SWIZZLE

#include "light_system.hpp"

#include "../physics/transform_comp.hpp"

#include <core/renderer/command_queue.hpp>
#include <core/renderer/primitives.hpp>
#include <core/renderer/texture_batch.hpp>
#include <core/renderer/graphics_ctx.hpp>


#define NUM_LIGHTS_MACRO 6

namespace lux {
namespace sys {
namespace light {

	using namespace renderer;

	namespace {
		static_assert(NUM_LIGHTS_MACRO==max_lights, "Update the NUM_LIGHTS_MACRO macro!");

		constexpr auto shadowed_lights = 2;
		constexpr auto shadowmap_size = 1024.f;
		constexpr auto shadowmap_rows = shadowed_lights;
	}

	Light_system::Light_system(
	             util::Message_bus& bus,
	             ecs::Entity_manager& entity_manager,
	             asset::Asset_manager& asset_manager,
	             renderer::Graphics_ctx&  graphics_ctx,
	             Rgb sun_light,
	             glm::vec3 sun_dir,
	             float ambient_brightness,
	             Rgba background_tint)
	    : _mailbox(bus),
	      _graphics_ctx(graphics_ctx),
	      _lights(entity_manager.list<Light_comp>()),
	      _shadowcaster_queue(1),
	      _shadowcaster_batch(_shadowcaster_shader, 64),
	      _occlusion_map    {Framebuffer(shadowmap_size,shadowmap_size, false, false),
	                         Framebuffer(shadowmap_size,shadowmap_size, false, false)},
	      _shadow_map       (shadowmap_size/2.f,shadowmap_rows, false, true),
	      _sun_light(sun_light),
	      _sun_dir(glm::normalize(sun_dir)),
	      _ambient_brightness(ambient_brightness),
	      _background_tint(background_tint) {


		entity_manager.register_component_type<Light_comp>();

		_shadowcaster_shader.attach_shader(asset_manager.load<Shader>("vert_shader:sprite"_aid))
		            .attach_shader(asset_manager.load<Shader>("frag_shader:sprite_shadow"_aid))
		            .bind_all_attribute_locations(renderer::sprite_layout)
		            .build()
		            .uniforms(make_uniform_map(
		                "albedo_tex", int(Texture_unit::color),
		                "height_tex", int(Texture_unit::height),
		                "model", glm::mat4()
		            ));

		_shadowmap_shader.attach_shader(asset_manager.load<Shader>("vert_shader:shadowmap"_aid))
		            .attach_shader(asset_manager.load<Shader>("frag_shader:shadowmap"_aid))
		            .bind_all_attribute_locations(renderer::simple_vertex_layout)
		            .build()
		            .uniforms(make_uniform_map(
		                "occlusions", 0,
		                "shadowmap_size", shadowmap_size
		            ));

		_finalize_shader.attach_shader(asset_manager.load<Shader>("vert_shader:shadowfinal"_aid))
		        .attach_shader(asset_manager.load<Shader>("frag_shader:shadowfinal"_aid))
		            .bind_all_attribute_locations(renderer::simple_vertex_layout)
		            .build()
		            .uniforms(make_uniform_map(
		                "shadowmaps_tex", 0
		            ));

		_blur_shader.attach_shader(asset_manager.load<Shader>("vert_shader:shadow_blur"_aid))
		        .attach_shader(asset_manager.load<Shader>("frag_shader:shadow_blur"_aid))
		            .bind_all_attribute_locations(renderer::simple_vertex_layout)
		            .build()
		            .uniforms(make_uniform_map(
		                "texture", 0,
		                "texture_size", glm::vec2(shadowmap_size,shadowmap_size)
		            ));
	}


	struct Light_info {
		const physics::Transform_comp* transform = nullptr;
		const Light_comp* light = nullptr;
		float score = 0.f;
		glm::vec2 flat_pos;

		bool operator<(const Light_info& rhs)const noexcept {
			return score>rhs.score;
		}
	};

	namespace {
		void fill_with_relevant_lights(const renderer::Camera& camera,
		                               Light_comp::Pool& lights,
		                               std::array<Light_info, max_lights>& out) {
			auto eye_pos = camera.eye_position();
			auto index = 0;

			for(Light_comp& light : lights) {
				auto& trans = light.owner().get<physics::Transform_comp>().get_or_throw();

				auto r = light.radius().value();
				auto dist = glm::distance2(remove_units(trans.position()), eye_pos);

				auto score = 1.f/dist;
				if(dist<10.f)
					score += glm::clamp(r/2.f + glm::length2(light.color())/4.f, -0.001f, 0.001f) + (light.shadowcaster() ? 1.f : 0.f);

				if(index<max_lights) {
					out[index].transform = &trans;
					out[index].light = &light;
					out[index].score = score;
					index++;

				} else { // too many lights => select brightest/closest
					auto min = std::min_element(out.begin(), out.end(),
												[](auto& a, auto& b){return a.score<b.score;});

					if(score>min->score) {
						min->transform = &trans;
						min->light = &light;
						min->score = score;
					}
				}
			}

			std::sort(out.begin(), out.end());
		}

		void bind_light_positions(renderer::Shader_program& prog, gsl::span<Light_info> lights) {
#define SET_LIGHT_UNIFORM(I) prog.set_uniform("light_positions["#I"]", lights[I].flat_pos);
			M_REPEAT(NUM_LIGHTS_MACRO, SET_LIGHT_UNIFORM);
#undef SET_LIGHT_UNIFORM
		}
	}
	void Light_system::prepare_draw(renderer::Command_queue& queue,
	                                const renderer::Camera& camera) {

		std::array<Light_info, max_lights> lights{};
		fill_with_relevant_lights(camera, _lights, lights);

		auto uniforms = queue.shared_uniforms();
		_setup_uniforms(*uniforms, camera, lights);

		_draw_occlusion_map(uniforms);
		_draw_shadow_map(lights);
		_draw_final(lights);
		_blur_shadows();

		_occlusion_map[0].bind((int) Texture_unit::shadowmaps);
	}
	void Light_system::_draw_shadow_map(gsl::span<Light_info> lights) {
		_shadowmap_shader.bind();

		bind_light_positions(_shadowmap_shader, lights);

		auto fbo_cleanup = Framebuffer_binder{_shadow_map};
		_shadow_map.clear();

		renderer::draw_fullscreen_quad(_occlusion_map[0]);
	}

	void Light_system::_draw_final(gsl::span<Light_info> lights) {
		_finalize_shader.bind();
		bind_light_positions(_finalize_shader, lights);

		auto fbo_cleanup = Framebuffer_binder{_occlusion_map[0]};
		auto depth_cleanup = renderer::Disable_depthtest{};
		auto blend_cleanup = renderer::Disable_blend{};

		renderer::draw_fullscreen_quad(_shadow_map);
	}
	void Light_system::_blur_shadows() {
		auto passes = static_cast<int>(std::ceil(16.f*_graphics_ctx.settings().shadow_softness));

		if(passes<=0.f) {
			return;
		}

		auto depth_cleanup = renderer::Disable_depthtest{};
		auto blend_cleanup = renderer::Disable_blend{};
		_blur_shader.bind();

		for(auto i : util::range(passes*2)) {
			auto src = i%2;
			auto dest = src>0?0:1;

			auto fbo_cleanup = Framebuffer_binder{_occlusion_map[dest]};

			_blur_shader.set_uniform("horizontal", i%2==0);
			renderer::draw_fullscreen_quad(_occlusion_map[src], Texture_unit::temporary);
		}
	}

	void Light_system::update(Time) {
	}

	void Light_system::_draw_occlusion_map(std::shared_ptr<IUniform_map> uniforms) {
		_shadowcaster_queue.shared_uniforms(uniforms);

		auto fbo_cleanup = Framebuffer_binder{_occlusion_map[0]};
		_occlusion_map[0].clear();

		_shadowcaster_batch.flush(_shadowcaster_queue);
		_shadowcaster_queue.flush();
	}

	void Light_system::_setup_uniforms(IUniform_map& uniforms, const renderer::Camera& camera,
	                                   gsl::span<Light_info> lights) {


		auto view = camera.view();
		if(glm::length2(view[3].xy()-_light_cam_pos)>0.5f) {
			_light_cam_pos = view[3].xy();
		}
		view[3].x = _light_cam_pos.x;
		view[3].y = _light_cam_pos.y;
		view[3].z -= 2.f; // compensates for screen-space technique limitations
		auto vp = camera.proj() * view;
		uniforms.emplace("vp", vp);
		uniforms.emplace("vp_light", vp);

		uniforms.emplace("light_ambient",   _ambient_brightness);
		uniforms.emplace("light_sun.color", _sun_light);
		uniforms.emplace("light_sun.dir",   _sun_dir);
		uniforms.emplace("background_tint", _background_tint);

		for(Light_info& l : lights) {
			if(!l.transform) {
				l.flat_pos = glm::vec2(1000,1000);
				continue;
			}

			auto pos = remove_units(l.transform->position()) + l.light->offset();
			pos.z = 0.f;
			auto p = vp*glm::vec4(pos, 1.f);
			l.flat_pos = p.xy() / p.w;
		}

		// TODO: fade out light color, when they left the screen
#define SET_LIGHT_UNIFORMS(N) \
		if(lights[N].light) {\
			uniforms.emplace("light["#N"].pos", remove_units(lights[N].transform->position())+lights[N].transform->resolve_relative(lights[N].light->offset()));\
\
			uniforms.emplace("light["#N"].dir", lights[N].transform->rotation().value()\
			                                               + lights[N].light->_direction.value());\
\
			uniforms.emplace("light["#N"].angle", lights[N].light->_angle.value());\
			uniforms.emplace("light["#N"].color", lights[N].light->color()); \
			uniforms.emplace("light["#N"].factors", lights[N].light->_factors);\
		} else {\
			uniforms.emplace("light["#N"].color", glm::vec3(0,0,0));\
		}

		M_REPEAT(NUM_LIGHTS_MACRO, SET_LIGHT_UNIFORMS);
#undef SET_LIGHT_UNIFORMS
	}

}
}
}
