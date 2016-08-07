/** simple wrapper for OpenGL textures ***************************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "../utils/log.hpp"
#include "../asset/asset_manager.hpp"

namespace lux {
namespace renderer {

	struct Texture_loading_failed : public asset::Loading_failed {
		explicit Texture_loading_failed(const std::string& msg)noexcept : Loading_failed(msg){}
	};

	enum Texture_format {
		RGB, RGBA
	};

	class Texture {
		public:
			explicit Texture(std::vector<uint8_t> buffer, bool cubemap) throw(Texture_loading_failed);
			Texture(int width, int height, const uint8_t* data, Texture_format format);
			virtual ~Texture()noexcept;

			Texture& operator=(Texture&&)noexcept;
			Texture(Texture&&)noexcept;

			void bind(int index=0)const;

			auto clip_rect()const noexcept {return _clip;}

			auto width()const noexcept {return _width;}
			auto height()const noexcept {return _height;}

			auto unsafe_low_level_handle()const {
				return _handle;
			}

			Texture(const Texture&) = delete;
			Texture& operator=(const Texture&) = delete;

		protected:
			Texture(int width, int height, int bpp=8);
			Texture(const Texture&, glm::vec4 clip)noexcept;
			void _update(const Texture&, glm::vec4 clip);

			unsigned int _handle;
			bool         _cubemap = false;
			bool         _owner = true;
			int          _width, _height;
			glm::vec4    _clip {0,0,1,1};
	};
	using Texture_ptr = asset::Ptr<Texture>;

	class Atlas_texture;

	class Texture_atlas : public std::enable_shared_from_this<Texture_atlas> {
		public:
			Texture_atlas(asset::istream);
			Texture_atlas& operator=(Texture_atlas&&)noexcept;
			~Texture_atlas();

			auto get(const std::string& name)const -> std::shared_ptr<Texture>;

		private:
			friend class Atlas_texture;
			Texture_ptr _texture;
			std::unordered_map<std::string, glm::vec4> _sub_positions;
			mutable std::unordered_map<std::string, std::weak_ptr<Atlas_texture>> _subs;
	};
	using Texture_atlas_ptr = asset::Ptr<Texture_atlas>;


	class Framebuffer : public Texture {
		public:
			Framebuffer(int width, int height, bool depth_buffer, bool hdr);
			~Framebuffer()noexcept;

			Framebuffer& operator=(Framebuffer&&)noexcept;
			Framebuffer(Framebuffer&& s)noexcept;


			void clear(glm::vec3 color=glm::vec3(0,0,0));
			void bind_target();
			void unbind_target();
			void set_viewport();

		private:
			unsigned int _fb_handle;
			unsigned int _db_handle;
	};

	struct Framebuffer_binder {
		Framebuffer_binder(Framebuffer& fb);
		~Framebuffer_binder()noexcept;

		Framebuffer& fb;
		int old_viewport[4];
	};

} /* namespace renderer */


namespace asset {
	template<>
	struct Loader<renderer::Texture_atlas> {
		using RT = std::shared_ptr<renderer::Texture_atlas>;

		static RT load(istream in) throw(Loading_failed){
			return std::make_shared<renderer::Texture_atlas>(std::move(in));
		}

		static void store(ostream out, const renderer::Texture_atlas& asset) throw(Loading_failed) {
			FAIL("NOT IMPLEMENTED!");
		}
	};

	template<>
	struct Loader<renderer::Texture> {
		using RT = std::shared_ptr<renderer::Texture>;

		static RT load(istream in) throw(Loading_failed) {
			constexpr auto cube_aid = util::Str_id{"tex_cube"};
			return std::make_shared<renderer::Texture>(in.bytes(), in.aid().type()==cube_aid);
		}

		static void store(ostream out, const renderer::Texture& asset) throw(Loading_failed) {
			FAIL("NOT IMPLEMENTED!");
		}
	};

	template<>
	struct Interceptor<renderer::Texture> {
		static auto on_intercept(Asset_manager& manager, const AID& interceptor_aid,
		                         const AID& org_aid) throw(Loading_failed) {
			auto atlas = manager.load<renderer::Texture_atlas>(interceptor_aid);

			return atlas->get(org_aid.name());
		}
	};
}

}
