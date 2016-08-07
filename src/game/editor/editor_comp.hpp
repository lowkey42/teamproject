/** editor specific state ****************************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/renderer/texture.hpp>

#include <core/ecs/ecs.hpp>
#include <core/units.hpp>


namespace lux {
namespace editor {

	class Blueprint_bar;

	class Editor_comp : public ecs::Component<Editor_comp> {
		public:
			static constexpr const char* name() {return "Editor";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Editor_comp(ecs::Entity& owner)noexcept
			  : Component(owner) {}

			auto bounds()const noexcept {return _bounds;}
			auto bounds(Position bounds) {_bounds = bounds;}

		private:
			friend class Blueprint_bar;

			Position _bounds;
	};

}
}
