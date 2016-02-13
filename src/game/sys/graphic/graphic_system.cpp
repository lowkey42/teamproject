#include "graphic_system.hpp"

#include <core/units.hpp>

#include "../physics/physics_comp.hpp"


namespace mo {
namespace sys {
namespace graphic {

	using namespace renderer;
	using namespace unit_literals;


	Graphic_system::Graphic_system(
	        util::Message_bus& bus,
	        ecs::Entity_manager& entity_manager,
	        sys::physics::Scene_graph& scene_graph,
	        asset::Asset_manager& asset_manager)
	    : _mailbox(bus),
	      _scene_graph(scene_graph),
	      _sprites(entity_manager.list<Sprite_comp>()),
	      _sprite_batch()
	{
		entity_manager.register_component_type<Sprite_comp>();

		_mailbox.subscribe_to<16, 128>([&](const State_change& e) {
			this->_on_state_change(e);
		});
	}

	void Graphic_system::draw(renderer::Command_queue& queue, const renderer::Camera& camera)const {
		glm::vec2 upper_left = camera.screen_to_world({camera.viewport().x, camera.viewport().y});
		glm::vec2 lower_right = camera.screen_to_world({camera.viewport().z, camera.viewport().w});

		_scene_graph.foreach_in_rect(upper_left, lower_right, [&](ecs::Entity& entity) {
			process(entity.get<physics::Transform_comp>(),
			        entity.get<Sprite_comp>())
			>> [&](const auto& trans, const auto& sp) {
				auto position = glm::vec3 {
					trans.position().x.value(),
					trans.position().y.value(),
					trans.layer()
				};

				_sprite_batch.insert(renderer::Sprite{position, trans.rotation(),
				                     remove_units(sp._size), glm::vec4{0,0,1,1},
				                     *sp._material});
			};
		});

		_sprite_batch.flush(queue);
	}

	void Graphic_system::update(Time dt) {
	}

	void Graphic_system::_on_state_change(const State_change& s) {
	}

}
}
}
