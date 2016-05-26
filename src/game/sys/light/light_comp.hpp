/** A simple point or spot light *********************************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


namespace lux {
namespace sys {
namespace light {

	struct Light_comp : public ecs::Component<Light_comp> {
		public:
			static constexpr const char* name() {return "Light";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Light_comp(ecs::Entity& owner);

			auto color()const noexcept {return _color*_color_factor;}
			auto radius()const noexcept {return _radius;}
			auto offset()const noexcept {return _offset;}
			auto brightness_factor(float f) {_color_factor = f;}
			auto shadowcaster()const noexcept {return _shadowcaster;}

		private:
			friend class Light_system;

			Angle _direction;
			Angle _angle;
			Rgb _color;
			glm::vec3 _factors {1,0,1};
			bool _shadowcaster = true;

			float _color_factor = 1.f;
			bool _radius_based;
			Distance _radius;
			glm::vec3 _offset;
	};

}
}
}
