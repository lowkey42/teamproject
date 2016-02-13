/*****************************************************************************\
 * batching sprite renderer                                                  *
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
#include "material.hpp"
#include "../asset/aid.hpp"

#include "../../core/units.hpp"

#include <vector>


namespace mo {
namespace renderer {

	class Command;
	class Command_queue;


	struct Sprite {
		glm::vec3 position;
		Angle rotation;
		glm::vec2 size;
		glm::vec4 uv;
		const renderer::Material* material = nullptr;

		Sprite() = default;
		Sprite(glm::vec3 position, Angle rotation, glm::vec2 size,
		       glm::vec4 uv, const renderer::Material& material)noexcept;
	};

	struct Sprite_vertex {
		glm::vec3 position;
		glm::vec2 uv;
		float rotation;
		const renderer::Material* material;

		Sprite_vertex(glm::vec3 pos, glm::vec2 uv_coords, float rotation, const renderer::Material*);

		bool operator<(const Sprite_vertex& rhs)const noexcept {
			return material < rhs.material;
		}
	};

	extern void init_sprite_renderer(asset::Asset_manager& asset_manager);

	class Sprite_batch {
		public:
			Sprite_batch(std::size_t expected_size=64);

			void insert(const Sprite& sprite);
			void flush(Command_queue&);

		private:
			using Vertex_citer = std::vector<Sprite_vertex>::const_iterator;

			std::vector<Sprite_vertex>    _vertices;
			std::vector<renderer::Object> _objects;
			std::size_t                   _free_obj = 0;

			void _draw(Command_queue&);
			auto _draw_part(Vertex_citer begin, Vertex_citer end) -> Command;
	};

}
}
