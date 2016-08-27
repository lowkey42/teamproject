/** A decal representation of an entity **************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/units.hpp>
#include <core/ecs/ecs.hpp>
#include <core/renderer/texture.hpp>


namespace lux {
namespace sys {
namespace graphic {

	class Decal_comp : public ecs::Component<Decal_comp> {
		public:
			static constexpr const char* name() {return "Decal";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Decal_comp(ecs::Entity& owner) : Component(owner) {}

		private:
			friend class Graphic_system;

			renderer::Texture_ptr _texture;
			glm::vec2 _size;
	};

}
}
}
