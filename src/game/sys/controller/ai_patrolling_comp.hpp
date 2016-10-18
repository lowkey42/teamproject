/** Simple Goomba-Like AI ****************************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/component.hpp>


namespace lux {
namespace sys {
namespace controller {

	class Ai_patrolling_comp : public ecs::Component<Ai_patrolling_comp> {
		public:
			static constexpr auto name() {return "Ai_patrolling";}
			friend void load_component(ecs::Deserializer& state, Ai_patrolling_comp&);
			friend void save_component(ecs::Serializer& state, const Ai_patrolling_comp&);

			Ai_patrolling_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner)
			    : Component(manager, owner) {}

		private:
			friend class Controller_system;

			glm::vec2 _velocity = {1.f, 0.f};
			float _max_distance = -1.f;
			bool _flip_horizontal_on_return = false;
			bool _flip_vertical_on_return = false;

			bool _moving_left = false;
			Position _start_position;
			bool _start_position_set = false;
	};

}
}
}
