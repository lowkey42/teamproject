/**************************************************************************\
 * terrain like transformable texture                                     *
 *                                               ___                      *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2016 Florian Oetke                                       *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include "texture.hpp"
#include "sprite_batch.hpp"

#include "../asset/asset_manager.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string>
#include <vector>
#include <stdexcept>


namespace mo {
namespace renderer {


	class Smart_texture {
		public:
			Smart_texture(Material_ptr material);

			auto points()const -> auto& {return _points;}

			void move_point(std::size_t i, glm::vec2 p);
			void insert_point(std::size_t i, glm::vec2 p);
			void erase_point(std::size_t i);


			void draw(glm::vec3 position, Sprite_batch&);

		private:
			Material_ptr _material;
			std::vector<glm::vec2> _points;
			std::vector<Sprite_vertex> _vertices;
			bool _dirty;

			void _update_vertices();
	};

}
}
