/*********************************************************************************\
 * Provides fast access to entities base on their position                       *
 *                                               ___                             *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___            *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|           *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \           *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/           *
 *                |___/                              |_|                         *
 *                                                                               *
 * Copyright (c) 2014 Florian Oetke                                              *
 *                                                                               *
 *  This file is part of MagnumOpus and distributed under the MIT License        *
 *  See LICENSE file for details.                                                *
\*********************************************************************************/

#pragma once

#include <functional>
#include <vector>
#include <array>
#include <glm/gtx/norm.hpp>

#include <core/utils/maybe.hpp>

#include "../../../core/utils/template_utils.hpp"
#include "transform_comp.hpp"


namespace mo {
namespace sys {
namespace physics {

	class Transform_system {
		public:
			Transform_system(ecs::Entity_manager& entity_manager);

			void update(Time dt){}
	};

	using Scene_graph = Transform_system;

}
}
}
