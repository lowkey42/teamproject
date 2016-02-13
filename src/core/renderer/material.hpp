/**************************************************************************\
 * simple wrapper for materials (collection of textures)                  *
 *                                               ___                      *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2014 Florian Oetke                                       *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "texture.hpp"
#include "../utils/log.hpp"
#include "../asset/asset_manager.hpp"

namespace mo {
namespace renderer {

	class Command;

	class Material {
		public:
			Material(asset::istream);

			void set_textures(Command&)const;

			auto albedo()const noexcept -> const Texture& {
				return *_albedo;
			}

		private:
			Texture_ptr _albedo;
			Texture_ptr _normal;
			Texture_ptr _roughness;
			Texture_ptr _metallic;
			Texture_ptr _emission;
			Texture_ptr _height;
	};
	using Material_ptr = asset::Ptr<Material>;

	extern void init_materials(asset::Asset_manager&);

}

namespace asset {
	template<>
	struct Loader<renderer::Material> {
		using RT = std::shared_ptr<renderer::Material>;

		static RT load(istream in) {
			return std::make_shared<renderer::Material>(std::move(in));
		}

		static void store(ostream out, const renderer::Texture_atlas& asset) {
			FAIL("NOT IMPLEMENTED!");
		}
	};
}

}
