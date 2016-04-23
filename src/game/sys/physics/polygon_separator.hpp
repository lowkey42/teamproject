/** Seperates a complex polygon into simple b2Shapes *************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <glm/vec2.hpp>
#include <Box2D/Box2D.h>

#include <vector>


namespace lux {
namespace sys {
namespace physics {

	extern void create_polygons(const std::vector<glm::vec2> points,
	                            b2Body& body, b2FixtureDef fixture);

}
}
}
