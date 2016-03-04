/*****************************************************************************\
 * basic structures and types                                                *
 *        ______   ______   __  __   ______                                  *
 *       /_____/\ /_____/\ /_/\/_/\ /_____/\                                 *
 *       \:::_ \ \\:::_ \ \\:\ \:\ \\::::_\/_                                *
 *        \:\ \ \ \\:(_) \ \\:\ \:\ \\:\/___/\                               *
 *         \:\ \ \ \\: ___\/ \:\ \:\ \\_::._\:\                              *
 *          \:\_\ \ \\ \ \    \:\_\:\ \ /____\:\                             *
 *           \_____\/ \_\/     \_____\/ \_____\/                             *
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "../units.hpp"
#include "shader.hpp"
#include "vertex_object.hpp"
#include "texture.hpp"

namespace mo {
namespace renderer {

	class Command_queue;
	class Texture;

	struct Simple_vertex {
		glm::vec2 xy;
		glm::vec2 uv;
		Simple_vertex(glm::vec2 xy, glm::vec2 uv) : xy(xy), uv(uv) {}
	};
	extern Vertex_layout simple_vertex_layout;

	extern void draw_dashed_line(renderer::Command_queue& queue,
	                             glm::vec2 p1, glm::vec2 p2, float dash_len, Rgba color);

	extern void init_primitives(asset::Asset_manager& asset_manager);

}
}
