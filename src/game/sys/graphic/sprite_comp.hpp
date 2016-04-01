/** A sprite representation of an entity *************************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/units.hpp>
#include <core/ecs/ecs.hpp>
#include <core/renderer/material.hpp>

namespace lux {
namespace sys {
namespace graphic {

	class Sprite_comp : public ecs::Component<Sprite_comp> {
		public:
			static constexpr const char* name() {return "Sprite";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Sprite_comp(ecs::Entity& owner, renderer::Material_ptr material = {}) :
				Component(owner), _material(material) {}

			auto size()const noexcept {return _size;}
			void size(glm::vec2 size) {_size = size;}

		private:
			friend class Graphic_system;

			renderer::Material_ptr _material;
			glm::vec2 _size;
			bool _shadowcaster = true;
	};

}
}
}
