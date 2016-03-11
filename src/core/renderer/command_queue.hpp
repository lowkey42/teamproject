/**************************************************************************\
 * queue of delayed OpenGL operations                                     *
 *                                               ___                      *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2015 Florian Oetke & Sebastian Schalow                   *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include "uniform_map.hpp"
#include "vertex_object.hpp"
#include "shader.hpp"

#include "../engine.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <array>


namespace mo {
namespace renderer {

	class Texture;


	enum class Texture_unit {
		temporary = 0, //< may be rebound at any momemnt

		color     = 1,
		normal    = 2,
		emission  = 3,
		roughness = 4,
		metallic  = 5,
		height    = 6,

		shadowmap_1=7,
		shadowmap_2=8,
		shadowmap_3=9,
		shadowmap_4=10,

		environment = 11, //< used for reflections
		last_frame = 12 //< used for reflections
	};
	constexpr auto texture_units = static_cast<std::size_t>(Texture_unit::last_frame)+1;

	enum class Gl_option : unsigned int {
		blend       = 0b001,
		depth_test  = 0b010,
		depth_write = 0b100
	};
	using Gl_options = unsigned int;

	constexpr Gl_options operator|(Gl_option lhs, Gl_option rhs)noexcept {
		return static_cast<Gl_options>(lhs) | static_cast<Gl_options>(rhs);
	}
	constexpr Gl_options operator|(Gl_options lhs, Gl_option rhs)noexcept {
		return lhs | static_cast<Gl_options>(rhs);
	}
	constexpr bool operator&(Gl_options lhs, Gl_option rhs)noexcept {
		return lhs & static_cast<Gl_options>(rhs);
	}

	constexpr auto default_gl_options = Gl_option::blend|Gl_option::depth_test|Gl_option::depth_write;


	constexpr auto uniforms_per_command = static_cast<std::size_t>(6);
	constexpr auto uniform_size_per_command = sizeof(float)*4;
	using Cmd_uniform_map = Uniform_map<uniforms_per_command, uniform_size_per_command>;

	//< uniform_maps MUST NOT override other uniforms
	class Command {
		public:
			auto shader(Shader_program&) -> Command&;
			auto texture(Texture_unit, const Texture&) -> Command&;
			auto object(const Object&) -> Command&;
			auto order_dependent() -> Command&;
			auto require(Gl_option) -> Command&;
			auto require_not(Gl_option) -> Command&;
			auto ext_uniforms(const IUniform_map&) -> Command&;
			auto uniforms() -> Cmd_uniform_map&;

			bool operator<(const Command& rhs)const noexcept;

		private:
			friend class Command_queue;

			std::array<const Texture*, texture_units> _textures {};
			Shader_program* _shader = nullptr;
			Cmd_uniform_map _private_uniforms;
			const IUniform_map* _ext_uniforms = nullptr;
			const Object* _obj = nullptr;
			Gl_options _gl_options = default_gl_options;

			int _order_dependent = false;

			int _textures_hash = 0;

			void _update_hashes()noexcept;
	};

	inline auto create_command() -> Command {return Command{};}


	class Command_queue {
		public:
			Command_queue(std::size_t expected=64);

			auto shared_uniforms(std::shared_ptr<IUniform_map>) -> IUniform_map&;
			auto shared_uniforms() -> std::shared_ptr<IUniform_map>;

			void flush();
			void push_back(const Command& command);

		private:
			std::shared_ptr<IUniform_map> _shared_uniforms;

			std::vector<Command> _commands;
			std::vector<Command> _order_dependent_commands;

			void _execute_commands(const IUniform_map* shared_uniforms,
			                       Command& last, bool& is_first,
			                       std::vector<Command>& commands);
	};



}
}
