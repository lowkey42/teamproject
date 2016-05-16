/** An entity that can transform into a light-beam ***************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "light_tag_comps.hpp"

#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/ecs.hpp>


namespace lux {
namespace sys {
namespace gameplay {

	class Gameplay_system;

	class Enlightened_comp : public ecs::Component<Enlightened_comp> {
		private:
			enum class State {
				disabled, pending, canceling, activating, enabled
			};

		public:
			static constexpr const char* name() {return "Enlightened";}
			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			Enlightened_comp(ecs::Entity& owner);

			bool can_air_transform()const {return _air_transforms_left>0;}
			void start_transformation()  {_state = State::pending;}
			void finish_transformation() {_state = State::activating;}
			void cancel_transformation() {_state = State::canceling;}
			void direction(glm::vec2 dir) {_direction = dir;}
			auto direction() {return _direction;}

			auto disabled()const noexcept {return _state==State::disabled || _state==State::canceling;}
			auto enabled()const noexcept {return _state==State::enabled;}
			bool was_light()const noexcept {return _was_light;}

			bool smash() {
				if(!_smashed){
					_smashed = true;
					return true;
				} else {
					return false;
				}
			}

		private:
			friend class Gameplay_system;

			float _velocity = 10.f;
			int _air_transformations = 0;
			float _radius = 1.f;
			float _final_booster_time = 0.f;
			float _max_air_time = -1.f;
			Light_color _color;

			bool _was_light = false; //< state after the last update
			State _state = State::disabled; //< state change request
			glm::vec2 _direction {0,-1};
			int _air_transforms_left=0;
			Time _air_time{};
			Time _final_booster_left{};
			bool _smashed = false;
	};

}
}
}
