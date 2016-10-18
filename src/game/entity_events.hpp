/** Events used for communication between components and entities ************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/ecs/ecs.hpp>
#include <core/utils/str_id.hpp>
#include <core/units.hpp>

namespace lux {

	struct State_change {
		ecs::Entity_facet entity;
		util::Str_id id;
		float magnitute;
		bool continuous;
		int priority; //< 0=highest, 100=lowest
	};

}
