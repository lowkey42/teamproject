/** simple wrapper for OpenGL shader/programs ********************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "../asset/asset_manager.hpp"

#include <glm/glm.hpp>

#include <gsl.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace lux {
namespace renderer {

	class Shader_program;

	struct Shader_compiler_error : public asset::Loading_failed {
		explicit Shader_compiler_error(const std::string& msg)noexcept : asset::Loading_failed(msg){}
	};

	enum class Shader_type {
		fragment,
		vertex
	};

	class Shader {
		public:
			Shader(Shader_type type, const std::string& source, const std::string& name)throw(Shader_compiler_error);
			~Shader()noexcept;

			Shader& operator=(Shader&&);

		private:
			friend class Shader_program;
			unsigned int _handle;
			std::string _name;
			mutable std::vector<Shader_program*> _attached_to;

			void _on_attach(Shader_program* prog)const;
			void _on_detach(Shader_program* prog)const;
	};

	class Vertex_layout;

	struct IUniform_map;

	class Shader_program {
		public:
			Shader_program();
			Shader_program(Shader_program&&) = delete;
			~Shader_program()noexcept;

			Shader_program& operator=(Shader_program&&) = delete;

			Shader_program& attach_shader(std::shared_ptr<const Shader> shader);
			Shader_program& bind_all_attribute_locations(const Vertex_layout&);
			Shader_program& bind_attribute_location(const std::string& name, int l);
			Shader_program& build()throw(Shader_compiler_error);
			Shader_program& uniforms(std::unique_ptr<IUniform_map>&&);
			Shader_program& detach_all();


			Shader_program& bind();
			Shader_program& unbind();

			Shader_program& set_uniform(const char* name, int value);
			Shader_program& set_uniform(const char* name, float value);
			Shader_program& set_uniform(const char* name, const glm::vec2& value);
			Shader_program& set_uniform(const char* name, const glm::vec3& value);
			Shader_program& set_uniform(const char* name, const glm::vec4& value);
			Shader_program& set_uniform(const char* name, const glm::mat2& value);
			Shader_program& set_uniform(const char* name, const glm::mat3& value);
			Shader_program& set_uniform(const char* name, const glm::mat4& value);

		private:
			template<class T>
			struct Uniform_entry {
				int handle;
				T last_value;
			};
			template<class T>
			using Uniform_cache = std::unordered_map<std::string, Uniform_entry<T>>;

			unsigned int _handle;
			std::vector<std::shared_ptr<const Shader>> _attached_shaders;
			std::unique_ptr<IUniform_map> _uniforms;

			Uniform_cache<int> _uniform_locations_int;
			Uniform_cache<float> _uniform_locations_float;
			Uniform_cache<glm::vec2> _uniform_locations_vec2;
			Uniform_cache<glm::vec3> _uniform_locations_vec3;
			Uniform_cache<glm::vec4> _uniform_locations_vec4;
			Uniform_cache<glm::mat2> _uniform_locations_mat2;
			Uniform_cache<glm::mat3> _uniform_locations_mat3;
			Uniform_cache<glm::mat4> _uniform_locations_mat4;
	};

} /* namespace renderer */

namespace asset {
	template<>
	struct Loader<renderer::Shader> {
		using RT = std::shared_ptr<renderer::Shader>;

		static RT load(istream in) throw(Loading_failed){
			switch(in.aid().type()) {
				case "frag_shader"_strid:
					return std::make_shared<renderer::Shader>(renderer::Shader_type::fragment, in.content(), in.aid().str());

				case "vert_shader"_strid:
					return std::make_shared<renderer::Shader>(renderer::Shader_type::vertex, in.content(), in.aid().str());

				default:
					break;
			}

			throw Loading_failed("Unsupported assetId for shader: "+in.aid().str());
		}

		static void store(istream out, const renderer::Shader&) throw(Loading_failed) {
			throw Loading_failed("Saving shaders is not supported!");
		}
	};
}
}
