/** A sprite representation of an entity *************************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/units.hpp>
#include <core/ecs/ecs.hpp>
#include <core/renderer/smart_texture.hpp>

namespace lux {
namespace sys {
namespace graphic {

	class Terrain_comp : public ecs::Component<Terrain_comp> {
		public:
			static constexpr const char* name() {return "Terrain";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Terrain_comp(ecs::Entity& owner, renderer::Material_ptr material = {}) :
				Component(owner), _smart_texture(material) {}

			auto& smart_texture()noexcept {return _smart_texture;}
			auto& smart_texture()const noexcept {return _smart_texture;}

		private:
			friend class Graphic_system;

			renderer::Smart_texture _smart_texture;
	};

}
}
}
