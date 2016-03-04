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

#include "editor_comp.hpp"

#include <core/asset/asset_manager.hpp>

#include <core/renderer/texture.hpp>
#include <core/renderer/camera.hpp>
#include <core/renderer/texture_batch.hpp>

#include <core/ecs/ecs.hpp>


namespace mo {
	namespace renderer {
		class Command_queue;
	}

namespace sys {
namespace editor {

	struct Entity_blueprint_info {
		std::string id;
		renderer::Texture_ptr icon;
	};
	struct Editor_conf;

	class Editor_system {
		public:
			Editor_system(ecs::Entity_manager& entity_manager, asset::Asset_manager& assets);

			void draw_blueprint_list(renderer::Command_queue& queue, glm::vec2 offset);
			auto find_blueprint(glm::vec2 click_position,
			                    glm::vec2 offset)const -> util::maybe<const Entity_blueprint_info&>;

			// TODO: set category
			// TODO: list categories

		private:
			asset::Ptr<Editor_conf> _conf;
			std::string _current_category;

			renderer::Texture_batch _icon_batch;
	};

}
}
}
