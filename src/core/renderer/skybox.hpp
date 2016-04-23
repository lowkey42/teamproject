/** Renders a simple skybox **************************************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "texture.hpp"
#include "shader.hpp"
#include "vertex_object.hpp"

#include "command_queue.hpp"

namespace lux {
namespace renderer {

	class Skybox {
		public:
			Skybox(asset::Asset_manager&, const asset::AID& sky);

			void draw(Command_queue&)const;

			void texture(Texture_ptr tex);

		private:
			mutable Shader_program _prog;
			Object         _obj;
			Texture_ptr    _tex;
	};

}
}
