/*****************************************************************************\
 * batching texture renderer                                                 *
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

#include "vertex_object.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "../asset/aid.hpp"

#include "../../core/units.hpp"

#include <vector>


namespace mo {
namespace renderer {

	class Command;
	class Command_queue;

	struct Texture_Vertex {
		glm::vec2 position;
		glm::vec2 uv;
		const renderer::Texture* tex;

		Texture_Vertex(glm::vec2 pos, glm::vec2 uv_coords, const renderer::Texture*);

		bool operator<(const Texture_Vertex& rhs)const noexcept {
			return tex < rhs.tex;
		}
	};

	extern void init_texture_renderer(asset::Asset_manager& asset_manager);

	extern void draw_fullscreen_quad(const renderer::Texture&);


	class Texture_batch {
		public:
			Texture_batch(std::size_t expected_size=64);

			void insert(const Texture& texture, glm::vec2 pos, glm::vec2 size);
			void flush(Command_queue&);

		private:

			using Vertex_citer = std::vector<Texture_Vertex>::const_iterator;

			std::vector<Texture_Vertex>   _vertices;
			std::vector<renderer::Object> _objects;
			std::size_t                   _free_obj = 0;

			void _draw(Command_queue&);
			auto _draw_part(Vertex_citer begin, Vertex_citer end) -> Command;
	};

}
}
