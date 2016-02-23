/**************************************************************************\
 * a simple point or spot light                                           *
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

#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


namespace mo {
namespace sys {
namespace light {

	struct Light_comp : public ecs::Component<Light_comp> {
		public:
			static constexpr const char* name() {return "Light";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Light_comp(ecs::Entity& owner);

			auto color()const noexcept {return _color;}
			auto radius()const noexcept {return _radius;}

		private:
			friend class Light_system;

			Angle _direction;
			Angle _angle;
			Rgb _color;
			glm::vec3 _factors {1,0,1};

			bool _radius_based;
			Distance _radius;
	};

}
}
}
