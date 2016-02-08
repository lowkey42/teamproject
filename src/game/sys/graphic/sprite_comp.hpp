/**************************************************************************\
 *	sprite_comp.hpp	- Component class for Sprites                         *
 *                                                ___                     *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2015 Florian Oetke                                       *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include <core/units.hpp>
#include <core/ecs/ecs.hpp>
#include <core/renderer/texture.hpp>

namespace mo {
namespace sys {
namespace graphic {

	class Sprite_comp : public ecs::Component<Sprite_comp> {
		public:
			static constexpr const char* name() {return "Sprite";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Sprite_comp(ecs::Entity& owner, renderer::Texture_ptr texture = {}) :
				Component(owner), _texture(texture) {}

		private:
			friend class Graphic_system;

			renderer::Texture_ptr _texture;
			Position _size;
	};

}
}
}
