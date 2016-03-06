/**************************************************************************\
 * editor specific state                                                  *
 *                                               ___                      *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2014 Florian Oetke                                       *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include <core/renderer/texture.hpp>

#include <core/ecs/ecs.hpp>
#include <core/units.hpp>


namespace mo {
namespace sys {
namespace editor {

	class Editor_system;

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
			friend class Editor_system;

			Position _bounds;
	};

}
}
}
