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
#include <core/renderer/sprite_animation.hpp>


namespace lux {
namespace sys {
namespace graphic {

	inline auto rgb_to_hue(Rgb c) {
		auto min = std::min({c.r, c.g, c.b});
		auto max = std::max({c.r, c.g, c.b});
		auto h = 0.f;

		if(max==c.r)      h = (c.g-c.b)/(max-min);
		else if(max==c.g) h = 2.0 + (c.b-c.r)/(max-min);
		else if(max==c.b) h = 4.0 + (c.r-c.g)/(max-min);

		h*=60.f;
		if(h<0)
			h+=360.f;
		return Angle::from_degrees(h);
	}

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

			void hue_change_replacement(Angle a) {
				_hue_change_replacement = a;
			}

		private:
			friend class Graphic_system;

			renderer::Material_ptr _material;
			glm::vec2 _size;
			bool _shadowcaster = true;
			float _shadow_receiver = 1.f;
			float _decals_intensity = 0.f;
			Angle _hue_change_target {0};
			Angle _hue_change_replacement {0};
	};

	class Anim_sprite_comp : public ecs::Component<Anim_sprite_comp> {
		public:
			static constexpr const char* name() {return "Anim_sprite";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Anim_sprite_comp(ecs::Entity& owner);

			auto size()const noexcept {return _size;}
			void size(glm::vec2 size) {_size = size;}

			auto& state()      noexcept {return _anim_state;}
			auto& state()const noexcept {return _anim_state;}

			void play(renderer::Animation_clip_id id, float speed=1.f);
			void play_if(renderer::Animation_clip_id expected,
			             renderer::Animation_clip_id id, float speed=1.f);
			void play_next(renderer::Animation_clip_id id);
			auto playing()const noexcept -> util::maybe<renderer::Animation_clip_id>;

			void hue_change_replacement(Angle a) {
				_hue_change_replacement = a;
			}

		private:
			friend class Graphic_system;

			renderer::Sprite_animation_state _anim_state;
			glm::vec2 _size;
			bool _shadowcaster = true;
			float _shadow_receiver = 1.f;
			float _decals_intensity = 0.f;
			Angle _hue_change_target {0};
			Angle _hue_change_replacement {0};
	};

}
}
}
