#include "types.hpp"

#include "ecs.hpp"

#include "../utils/string_utils.hpp"

#include <string>


namespace lux {
namespace ecs {

	namespace detail {
		extern Component_type id_generator() {
			static auto next_id = static_cast<Component_type>(0);
			return ++next_id;
		}
	}

	auto get_entity_id(Entity_handle h, Entity_manager& manager) -> Entity_id {
		if(manager.validate(h)) {
			return h.id();
		} else {
			return invalid_entity_id;
		}
	}
	auto entity_name(Entity_handle h) -> std::string {
		return util::to_string(h.id()) + ":" + util::to_string<int>(h.revision());
	}

	Entity_facet::Entity_facet(Entity_manager& manager, Entity_handle owner)
	    : _manager(&manager), _owner(owner) {
		INVARIANT(valid(), "Created Entity_facet for invalid entity: "<<entity_name(owner));
	}
	auto Entity_facet::valid()const noexcept -> bool {
		return _manager && _manager->validate(_owner);
	}

}
}
