/** A particle emmiter attached to an entity *********************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/units.hpp>
#include <core/ecs/component.hpp>
#include <core/renderer/particles.hpp>


namespace lux {
namespace sys {
namespace graphic {

	class Particle_comp : public ecs::Component<Particle_comp> {
		public:
			static constexpr const char* name() {return "Particle";}
			friend void load_component(ecs::Deserializer& state, Particle_comp&);
			friend void save_component(ecs::Serializer& state, const Particle_comp&);

			Particle_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner,
			              renderer::Particle_emitter_ptr e = {})
			    : Component(manager, owner), _emitters{{e}} {}

			void add(renderer::Particle_type_id id);
			void remove(renderer::Particle_type_id id);

			void hue_change(Angle a) {
				_hue_change = a;
			}

		private:
			friend class Graphic_system;

			glm::vec3 _offset;
			Angle _hue_change;

			std::array<renderer::Particle_type_id, 3> _add_queue;
			std::array<renderer::Particle_emitter_ptr, 3> _emitters;
	};

}
}
}
