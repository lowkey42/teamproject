#include "input_controller_comp.hpp"

namespace lux {
namespace sys {
namespace controller {

	void Input_controller_comp::load(sf2::JsonDeserializer& state,
	                                 asset::Asset_manager& asset_mgr) {
		state.read_lambda([](auto&){return false;});
		// TODO
	}

	void Input_controller_comp::save(sf2::JsonSerializer& state)const {
		state.write_virtual();
		// TODO
	}

	Input_controller_comp::Input_controller_comp(ecs::Entity& owner) : Component(owner) {
		// TODO
	}

}
}
}
