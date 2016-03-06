/**************************************************************************\
 *	Sprite Component Management System                                    *
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

#include "../../entity_events.hpp"

#include <core/renderer/sprite_batch.hpp>
#include <core/renderer/camera.hpp>
#include <core/utils/messagebus.hpp>

#include "sprite_comp.hpp"


namespace mo {
namespace sys {
namespace graphic {

	class Graphic_system {
		public:
			Graphic_system(util::Message_bus& bus,
			               ecs::Entity_manager& entity_manager,
			               asset::Asset_manager& asset_manager);

			void draw(renderer::Command_queue&, const renderer::Camera& camera)const;
			void draw_shadowcaster(renderer::Sprite_batch&, const renderer::Camera& camera)const;
			void update(Time dt);

		private:
			void _on_state_change(const State_change&);


			util::Mailbox_collection _mailbox;
			Sprite_comp::Pool& _sprites;

			mutable renderer::Sprite_batch _sprite_batch;
	};

	extern void scale_entity(ecs::Entity&, float factor);

}
}
}
