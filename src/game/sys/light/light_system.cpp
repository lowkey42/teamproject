#define GLM_SWIZZLE

#include "light_system.hpp"

#include <core/renderer/command_queue.hpp>


namespace mo {
namespace sys {
namespace light {

	Light_system::Light_system(
	             util::Message_bus& bus,
	             ecs::Entity_manager& entity_manager,
	             physics::Scene_graph& scene_graph,
	             Rgb sun_light,
	             Angle sun_dir,
	             Rgb ambient_light)
	    : _mailbox(bus),
	      _scene_graph(scene_graph),
	      _lights(entity_manager.list<Light_comp>()),
	      _sun_light(sun_light),
	      _sun_dir(sun_dir),
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
		                               physics::Scene_graph& scene_graph,
		                               std::array<Light_info, max_lights>& out) {
			auto index = 0;
			auto area = camera.area();

			scene_graph.foreach_in_rect(area.xy(), area.xy(), [&](ecs::Entity& entity) {
				process(entity.get<physics::Transform_comp>(),
				        entity.get<Light_comp>())
				>> [&](const auto& trans, const auto& light) {
					auto dist = glm::distance2(remove_units(trans.position()), camera.position())
					            / camera.viewport().w;
					auto score = glm::length2(light.color()) * dist + light.radius().value()/camera.viewport().w;

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
				};
			});
		}
	}
	void Light_system::draw(renderer::Command_queue& queue, const renderer::Camera& camera)const {
		std::array<Light_info, max_lights> lights;

		fill_with_relevant_lights(camera, _scene_graph, lights);

		// TODO: draw shadowmaps if required

		queue.shared_uniforms().emplace("light_ambient",   _ambient_light);
		queue.shared_uniforms().emplace("light_sun.color", _sun_light);
		queue.shared_uniforms().emplace("light_sun.dir",   _ambient_light);

#define SET_LIGHT_UNIFORMS(N) \
		if(lights[N].light) {\
			queue.shared_uniforms().emplace("light_"#N".pos", glm::vec3{\
			                                    lights[N].transform->position().x.value(),\
			                                    lights[N].transform->position().y.value(),\
			                                    lights[N].transform->layer()});\
\
			queue.shared_uniforms().emplace("light_"#N".dir", lights[N].transform->rotation().value()\
			                                               + lights[N].light->_direction.value());\
\
			queue.shared_uniforms().emplace("light_"#N".angle", lights[N].light->_angle.value());\
			queue.shared_uniforms().emplace("light_"#N".color", lights[N].light->_color);\
			queue.shared_uniforms().emplace("light_"#N".factor", lights[N].light->_falloff_factor);\
		}

		SET_LIGHT_UNIFORMS(0)
		SET_LIGHT_UNIFORMS(1)
		SET_LIGHT_UNIFORMS(2)
		SET_LIGHT_UNIFORMS(3)
		SET_LIGHT_UNIFORMS(4)
		SET_LIGHT_UNIFORMS(5)

#undef SET_LIGHT_UNIFORMS
	}
	void Light_system::update(Time) {
	}

}
}
}
