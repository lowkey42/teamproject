#define GLM_SWIZZLE

#include "light_system.hpp"

#include "../physics/transform_comp.hpp"

#include <core/renderer/command_queue.hpp>


namespace mo {
namespace sys {
namespace light {

	Light_system::Light_system(
	             util::Message_bus& bus,
	             ecs::Entity_manager& entity_manager,
	             Rgb sun_light,
	             glm::vec3 sun_dir,
	             Rgb ambient_light)
	    : _mailbox(bus),
	      _lights(entity_manager.list<Light_comp>()),
	      _sun_light(sun_light),
	      _sun_dir(glm::normalize(sun_dir)),
	      _ambient_light(ambient_light) {

		entity_manager.register_component_type<Light_comp>();

	}

	namespace {

		struct Light_info {
			const physics::Transform_comp* transform = nullptr;
			const Light_comp* light = nullptr;
			float score = 0.f;
		};

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
	void Light_system::draw(renderer::Command_queue& queue, const renderer::Camera& camera)const {
		std::array<Light_info, max_lights> lights{};

		fill_with_relevant_lights(camera, _lights, lights);

		// TODO: draw shadowmaps if required

		queue.shared_uniforms().emplace("light_ambient",   _ambient_light);
		queue.shared_uniforms().emplace("light_sun.color", _sun_light);
		queue.shared_uniforms().emplace("light_sun.dir",   _sun_dir);

#define SET_LIGHT_UNIFORMS(N) \
		if(lights[N].light) {\
			queue.shared_uniforms().emplace("light["#N"].pos", remove_units(lights[N].transform->position()));\
\
			queue.shared_uniforms().emplace("light["#N"].dir", lights[N].transform->rotation().value()\
			                                               + lights[N].light->_direction.value());\
\
			queue.shared_uniforms().emplace("light["#N"].angle", lights[N].light->_angle.value());\
			queue.shared_uniforms().emplace("light["#N"].color", lights[N].light->_color);\
			queue.shared_uniforms().emplace("light["#N"].factors", lights[N].light->_factors);\
		} else {\
			queue.shared_uniforms().emplace("light["#N"].color", glm::vec3(0,0,0));\
		}

		SET_LIGHT_UNIFORMS(0)
		SET_LIGHT_UNIFORMS(1)
		SET_LIGHT_UNIFORMS(2)
		SET_LIGHT_UNIFORMS(3)

#undef SET_LIGHT_UNIFORMS
	}
	void Light_system::update(Time) {
	}

}
}
}
