/** simple wrapper for materials (collection of textures) ********************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "texture.hpp"
#include "../utils/log.hpp"
#include "../asset/asset_manager.hpp"

namespace lux {
namespace renderer {

	class Command;

	class Material {
		public:
			Material(asset::istream);

			void set_textures(Command&)const;

			auto albedo()const noexcept -> const Texture& {
				return *_albedo;
			}
			auto alpha()const noexcept {return _alpha;}

		private:
			Texture_ptr _albedo;
			Texture_ptr _normal;
			Texture_ptr _material; //< R:emmision, G:metallc, B:roughness
			Texture_ptr _height;
			bool        _alpha = false;
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
