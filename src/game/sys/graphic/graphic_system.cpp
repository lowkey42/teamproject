#include "graphic_system.hpp"

#include "../physics/transform_comp.hpp"

#include <core/units.hpp>


namespace mo {
namespace sys {
namespace graphic {

	using namespace renderer;
	using namespace unit_literals;


	Graphic_system::Graphic_system(
	        util::Message_bus& bus,
	        ecs::Entity_manager& entity_manager,
	        asset::Asset_manager& asset_manager)
	    : _mailbox(bus),
	      _sprites(entity_manager.list<Sprite_comp>()),
	      _sprite_batch()
	{
		entity_manager.register_component_type<Sprite_comp>();

		_mailbox.subscribe_to<16, 128>([&](const State_change& e) {
			this->_on_state_change(e);
		});
	}

	void Graphic_system::draw(renderer::Command_queue& queue, const renderer::Camera& camera)const {
		for(Sprite_comp& sprite : _sprites) {
			auto& trans = sprite.owner().get<physics::Transform_comp>().get_or_throw();

			auto position = remove_units(trans.position());

			_sprite_batch.insert(renderer::Sprite{position, trans.rotation(),
			                     sprite._size, glm::vec4{0,0,1,1},
			                     *sprite._material});
		}

		_sprite_batch.flush(queue);
	}

	void Graphic_system::update(Time dt) {
	}

	void Graphic_system::_on_state_change(const State_change& s) {
	}

}
}
}
