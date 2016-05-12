/** batching sprite renderer *************************************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
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


namespace lux {
namespace renderer {

	class Command;
	class Command_queue;


	struct Sprite {
		glm::vec3 position;
		Angle rotation;
		glm::vec2 size;
		glm::vec4 uv;
		float shadow_resistence;
		const renderer::Material* material = nullptr;

		Sprite() = default;
		Sprite(glm::vec3 position, Angle rotation, glm::vec2 size,
		       glm::vec4 uv, float shadow_resistence, const renderer::Material& material)noexcept;
	};

	struct Sprite_vertex {
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec4 uv_clip;
		glm::vec2 tangent;
		float shadow_resistence;
		const renderer::Material* material;

		Sprite_vertex() : shadow_resistence(false), material(nullptr) {}
		Sprite_vertex(glm::vec3 pos, glm::vec2 uv_coords, glm::vec4 uv_clip,
		              glm::vec2 tangent, float shadow_resistence, const renderer::Material*);

		bool operator<(std::tuple<float&, const renderer::Material*> rhs)const noexcept {
			auto lhs_z = -std::floor(position.z*100.f);
			auto lhs_alpha = material ? material->alpha() : false;
			auto rhs_z = -std::floor(std::get<0>(rhs)*100.f);
			auto rhs_alpha = std::get<1>(rhs) ? std::get<1>(rhs)->alpha() : false;

			return std::tie(lhs_alpha, material, lhs_z)
			     < std::tie(rhs_alpha, std::get<1>(rhs), rhs_z);
		}
		bool operator<(const Sprite_vertex& rhs)const noexcept {
			auto lhs_z = -std::floor(position.z*100.f);
			auto lhs_alpha = material ? material->alpha() : false;
			auto rhs_z = -std::floor(rhs.position.z*100.f);
			auto rhs_alpha = rhs.material ? rhs.material->alpha() : false;

			return std::tie(lhs_alpha, material, lhs_z)
			     < std::tie(rhs_alpha, rhs.material, rhs_z);
		}
	};

	extern Vertex_layout sprite_layout;

	extern void init_sprite_renderer(asset::Asset_manager& asset_manager);

	class Sprite_batch {
		public:
			Sprite_batch(std::size_t expected_size=64);
			Sprite_batch(Shader_program& shader, std::size_t expected_size=64);

			void insert(const Sprite& sprite);
			void insert(glm::vec3 position,
			            const std::vector<Sprite_vertex>& vertices);
			void flush(Command_queue&);

		private:
			using Vertex_citer = std::vector<Sprite_vertex>::const_iterator;
			using Vertex_iter = std::vector<Sprite_vertex>::iterator;

			Shader_program& _shader;

			std::vector<Sprite_vertex>    _vertices;
			std::vector<renderer::Object> _objects;
			std::size_t                   _free_obj = 0;

			void _draw(Command_queue&);
			auto _draw_part(Vertex_citer begin, Vertex_citer end) -> Command;
			auto _reserve_space(float z, const renderer::Material* material, std::size_t count) -> Vertex_iter;
	};

}
}
