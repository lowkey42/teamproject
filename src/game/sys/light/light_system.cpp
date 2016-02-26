#define GLM_SWIZZLE

#include "light_system.hpp"

#include "../physics/transform_comp.hpp"

#include <core/renderer/command_queue.hpp>
#include <core/renderer/primitives.hpp>
#include <core/renderer/texture_batch.hpp>


namespace mo {
namespace sys {
namespace light {

	using namespace mo::renderer;

	namespace {
		constexpr auto shadowmap_size = 1024.f;
		constexpr auto shadowmap_rows = max_lights*4.f;
	}

	Light_system::Light_system(
	             util::Message_bus& bus,
	             ecs::Entity_manager& entity_manager,
	             asset::Asset_manager& asset_manager,
	             Rgb sun_light,
	             glm::vec3 sun_dir,
	             Rgb ambient_light)
	    : _mailbox(bus),
	      _lights(entity_manager.list<Light_comp>()),
	      _shadowcaster_queue(1),
	      _shadowcaster_batch(_shadowcaster_shader, 64),
	      _occlusion_map    (shadowmap_size,shadowmap_size, false, false),
	      _shadow_map       (shadowmap_size,shadowmap_rows, false, true),
	      _sun_light(sun_light),
	      _sun_dir(glm::normalize(sun_dir)),
	      _ambient_light(ambient_light) {

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
	}


	struct Light_info {
		const physics::Transform_comp* transform = nullptr;
		const Light_comp* light = nullptr;
		float score = 0.f;
	};

	namespace {
		void fill_with_relevant_lights(const renderer::Camera& camera,
		                               Light_comp::Pool& lights,
		                               std::array<Light_info, max_lights>& out) {
			auto eye_pos = camera.eye_position();
			auto index = 0;

			for(Light_comp& light : lights) {
				auto& trans = light.owner().get<physics::Transform_comp>().get_or_throw();

				auto dist = glm::distance2(remove_units(trans.position()), eye_pos);
				auto score = glm::length2(light.color()) * 1.f/dist;

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
		}
	}
	void Light_system::prepare_draw(renderer::Command_queue& queue,
	                                const renderer::Camera& camera) {
		std::array<Light_info, max_lights> lights{};

		fill_with_relevant_lights(camera, _lights, lights);

		auto uniforms = queue.shared_uniforms();

		_setup_uniforms(*uniforms, lights);


		auto camp = camera.eye_position();
		auto vp = glm::ortho(-50.f, 50.f, -50.f, 50.f, -10.f, 10.f)
		          * glm::translate(glm::vec3(camp.x,camp.y,0.f));

		uniforms->emplace("VP", vp);

		_draw_occlusion_map(uniforms);

		_shadowmap_shader.bind();
		auto transform = [&vp](auto* transform) {
			if(!transform) return glm::vec2(1000,0);
			return (vp*glm::vec4(remove_units(transform->position()), 1.f)).xy();
		};

		_shadowmap_shader.set_uniform("VP_inv", glm::inverse(vp));
		_shadowmap_shader.set_uniform("light_positions[0]", transform(lights[0].transform));
		_shadowmap_shader.set_uniform("light_positions[1]", transform(lights[1].transform));
		_shadowmap_shader.set_uniform("light_positions[2]", transform(lights[2].transform));
		_shadowmap_shader.set_uniform("light_positions[3]", transform(lights[3].transform));

		auto fbo_cleanup = Framebuffer_binder{_shadow_map};
		_shadow_map.clear();

		renderer::draw_fullscreen_quad(_occlusion_map);

		_shadow_map.bind((int) Texture_unit::shadowmap_1);
	}

	void Light_system::update(Time) {
	}

	void Light_system::_draw_occlusion_map(std::shared_ptr<IUniform_map> uniforms) {
		_shadowcaster_queue.shared_uniforms(uniforms);

		auto fbo_cleanup = Framebuffer_binder{_occlusion_map};
		_occlusion_map.clear();

		_shadowcaster_batch.flush(_shadowcaster_queue);
		_shadowcaster_queue.flush();
	}

	void Light_system::_setup_uniforms(IUniform_map& uniforms, gsl::span<Light_info> lights) {

		uniforms.emplace("light_ambient",   _ambient_light);
		uniforms.emplace("light_sun.color", _sun_light);
		uniforms.emplace("light_sun.dir",   _sun_dir);

#define SET_LIGHT_UNIFORMS(N) \
		if(lights[N].light) {\
			uniforms.emplace("light["#N"].pos", remove_units(lights[N].transform->position()));\
\
			uniforms.emplace("light["#N"].dir", lights[N].transform->rotation().value()\
			                                               + lights[N].light->_direction.value());\
\
			uniforms.emplace("light["#N"].angle", lights[N].light->_angle.value());\
			uniforms.emplace("light["#N"].color", lights[N].light->_color);\
			uniforms.emplace("light["#N"].factors", lights[N].light->_factors);\
		} else {\
			uniforms.emplace("light["#N"].color", glm::vec3(0,0,0));\
		}

		SET_LIGHT_UNIFORMS(0)
		SET_LIGHT_UNIFORMS(1)
		SET_LIGHT_UNIFORMS(2)
		SET_LIGHT_UNIFORMS(3)

#undef SET_LIGHT_UNIFORMS
	}

}
}
}
