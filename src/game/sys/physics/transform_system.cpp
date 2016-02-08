#include "transform_system.hpp"
#include "physics_comp.hpp"

#include<glm/gtc/constants.hpp>

namespace mo {
namespace sys {
namespace physics {
	using namespace std::placeholders;
	using namespace unit_literals;

	namespace {
		/// source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		uint32_t next_pot(uint32_t n) {
			n--;
			n |= n >> 1;
			n |= n >> 2;
			n |= n >> 4;
			n |= n >> 8;
			n |= n >> 16;
			n++;

			return n;
		}
		int calc_cell_size(Distance max_entity_size) {
			return next_pot(static_cast<uint32_t>(std::ceil(max_entity_size.value()*2)));
		}
	}

	Transform_system::Transform_system(
			ecs::Entity_manager& entity_manager, Distance max_entity_size)
		: util::slot<ecs::Component_event>(&Transform_system::_on_comp_event, this),
	      _max_entity_size(max_entity_size),
		  _cell_size(calc_cell_size(max_entity_size)),
		  _em(entity_manager), _pool(_em.list<Transform_comp>()) {

		_em.register_component_type<physics::Transform_comp>();

		connect(entity_manager.list<Transform_comp>());
	}

	void Transform_system::update(Time) {
		for(auto& c : _pool) {
			if(c._dirty!=Transform_comp::State::clear) {
				auto old_cell_idx = c._cell_idx;
				auto new_cell_idx = _get_cell_idx_for(c._position);

				if(old_cell_idx!=new_cell_idx || c._dirty == Transform_comp::State::uninitialized) {
					c._cell_idx = new_cell_idx;

					_ro_cell_for(old_cell_idx) >> [&](auto& cell) {
						cell.remove(c.owner());
					};

					auto& new_cell = _cell_for(new_cell_idx);
					new_cell.add(c.owner());
				}

				c._dirty = Transform_comp::State::clear;
			}
		}
	}

	void Transform_system::_on_comp_event(ecs::Component_event e) {
		e.handle.get<Transform_comp>().process([&](Transform_comp& trans){
			if(e.type!=ecs::Component_event_type::created) {
				_ro_cell_for(trans._cell_idx) >> [&](auto& cell) {
					cell.remove(trans.owner());
				};
			}
		});
	}
	void Transform_system::Cell_data::add(ecs::Entity& c) {
		entities.emplace_back(&c);
	}

	void Transform_system::Cell_data::remove(ecs::Entity& c) {
		using std::swap;

		auto found = std::find_if(entities.begin(), entities.end(),
								   [&](ecs::Entity* e){return e==&c;} );

		if(found!=entities.end()) {
			swap(*found, entities.back());
			entities.pop_back();
		}
	}

	void Transform_system::pre_reload() {
		_cells.clear();
		_min_cell_key = {0,0};
		_max_cell_key = {0,0};
	}

	void Transform_system::post_reload() {
	}

}
}
}
