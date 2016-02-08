#ifndef ANDROID
	#include <GL/glew.h>
	#include <GL/gl.h>
#else
	#include <GLES2/gl2.h>
#endif

#include "texture.hpp"

#include <SDL2/SDL.h>
#include <soil/SOIL2.h>
#include <glm/glm.hpp>


namespace mo {
namespace renderer {

	using namespace glm;

#define CLAMP_TO_EDGE 0x812F

	Texture::Texture(std::vector<uint8_t> buffer) throw(Texture_loading_failed) {
		_handle = SOIL_load_OGL_texture_from_memory
		(
			buffer.data(),
			buffer.size(),
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_INVERT_Y | SOIL_FLAG_MULTIPLY_ALPHA,
			&_width,
			&_height
		);

		if(!_handle)
			throw Texture_loading_failed(SOIL_last_result());

		glBindTexture(GL_TEXTURE_2D, _handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	Texture::Texture(int width, int height) : _width(width), _height(height) {
		glGenTextures( 1, &_handle );
		glBindTexture( GL_TEXTURE_2D, _handle );
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, CLAMP_TO_EDGE);
	}
	Texture::Texture(const Texture& base, glm::vec4 clip)noexcept
	    : _handle(base._handle),
	      _owner(false),
	      _width(base._width * (clip.z-clip.x)),
	      _height(base._height * (clip.w-clip.y)),
	      _clip(clip) {
	}
	void Texture::_update(const Texture& base, glm::vec4 clip) {
		INVARIANT(!_owner, "_update is only supported for non-owning textures!");

		_handle = base._handle;
		_width = base._width * (clip.z-clip.x);
		_height = base._height * (clip.w-clip.y);
		_clip = clip;
	}

	Texture::~Texture()noexcept {
		if(_handle!=0 && _owner)
			glDeleteTextures(1, &_handle);
	}

	Texture::Texture(Texture&& rhs)noexcept
	    : _handle(rhs._handle), _owner(rhs._owner), _width(rhs._width),
	      _height(rhs._height), _clip(rhs._clip) {

		rhs._handle = 0;
	}
	Texture& Texture::operator=(Texture&& s)noexcept {
		if(_handle!=0 && _owner)
			glDeleteTextures(1, &_handle);

		_owner = s._owner;
		_handle = s._handle;
		s._handle = 0;

		_width = s._width;
		_height = s._height;
		_clip = s._clip;

		return *this;
	}

	void Texture::bind(int index)const {
		static int current_texture_unit = -1;

		auto tex = GL_TEXTURE0+index;
		INVARIANT(tex<GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "to many textures");

		if(current_texture_unit!=index) {
			glActiveTexture(GL_TEXTURE0 + index);
			current_texture_unit = index;
		}
		glBindTexture(GL_TEXTURE_2D, _handle);
	}



	class Atlas_texture : public Texture {
		public:
			Atlas_texture(const std::string& name,
			              std::shared_ptr<const Texture_atlas>, glm::vec4 clip);
			~Atlas_texture();

			using Texture::_update;

		private:
			const std::string _name;
			std::shared_ptr<const Texture_atlas> _atlas;
	};

	namespace {
		struct Frame {
			std::string name;
			float x, y, width, height;
		};
		sf2_structDef(Frame, name,x,y,width,height)

		struct Atlas_def {
			std::string texture;
			std::vector<Frame> frames;
		};
		sf2_structDef(Atlas_def, texture,frames)
	}

	Texture_atlas::Texture_atlas(asset::istream stream) {
		Atlas_def def;
		sf2::deserialize_json(stream, [&](auto& msg, uint32_t row, uint32_t column) {
			ERROR("Error parsing TextureAtlas from "<<stream.aid().str()<<" at "<<row<<":"<<column<<": "<<msg);
		}, def);

		_texture = stream.manager().load<Texture>(asset::AID(def.texture));
		_sub_positions.reserve(def.frames.size());
		_subs.reserve(def.frames.size());

		auto tex_size = vec2{_texture->width(), _texture->height()};

		for(Frame& frame : def.frames) {
			auto clip = vec4 {
				frame.x/tex_size.x,
				frame.y/tex_size.y,
				(frame.x+frame.width)/tex_size.x,
				(frame.y+frame.height)/tex_size.y
			};
			_sub_positions.emplace(frame.name, clip);
		}
	}
	Texture_atlas& Texture_atlas::operator=(Texture_atlas&& rhs)noexcept {
		_texture = std::move(rhs._texture);
		_sub_positions = std::move(rhs._sub_positions);

		for(auto& sub : _subs) {
			auto sub_tex = sub.second.lock();
			INVARIANT(sub_tex, "Texture has not been removed from its atlas");

			auto iter = _sub_positions.find(sub.first);
			if(iter==_sub_positions.end()) {
				ERROR("Texture that has been removed from atlas is still in use!");
				sub_tex->_update(*_texture, sub_tex->clip_rect());
				continue;
			}

			sub_tex->_update(*_texture, iter->second);
		}

		return *this;
	}
	Texture_atlas::~Texture_atlas() {
		INVARIANT(_subs.empty(), "A texture from this atlas has not been deleted");
	}
	auto Texture_atlas::get(const std::string& name)const -> std::shared_ptr<Texture> {
		auto& weak_sub = _subs[name];
		auto sub = weak_sub.lock();

		if(sub) {
			INVARIANT(sub, "Texture has not been removed from its atlas");
			return sub;
		}

		auto iter = _sub_positions.find(name);
		if(iter==_sub_positions.end())
			throw Texture_loading_failed("Couldn't find texture \""+name+"\" in atlas \""+_texture.aid().str()+"\"");

		sub = std::make_shared<Atlas_texture>(name, shared_from_this(), iter->second);
		weak_sub = sub;
		return sub;
	}

	Atlas_texture::Atlas_texture(const std::string& name,
	                             std::shared_ptr<const Texture_atlas> atlas, glm::vec4 clip)
	    : Texture(*atlas->_texture, clip), _name(name), _atlas(atlas) {
	}

	Atlas_texture::~Atlas_texture() {
		if(_atlas)
			_atlas->_subs.erase(_name);
	}


	Framebuffer::Framebuffer(int width, int height, bool depth_buffer)
		: Texture(width, height), _fb_handle(0), _db_handle(0) {

		glGenFramebuffers(1, &_fb_handle);
		glBindFramebuffer(GL_FRAMEBUFFER, _fb_handle);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _handle, 0);

		if(depth_buffer) {
			glGenRenderbuffers(1, &_db_handle);
			glBindRenderbuffer(GL_RENDERBUFFER, _db_handle);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
									  GL_RENDERBUFFER, _db_handle);
		}

		INVARIANT(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE, "Couldn't create framebuffer!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	Framebuffer::Framebuffer(Framebuffer&& rhs)noexcept
		: Texture(std::move(rhs)), _fb_handle(rhs._fb_handle), _db_handle(rhs._db_handle) {

		rhs._fb_handle = 0;
		rhs._db_handle = 0;
	}
	Framebuffer::~Framebuffer()noexcept {
		if(_fb_handle)
			glDeleteFramebuffers(1, &_fb_handle);

		if(_db_handle)
			glDeleteRenderbuffers(1, &_db_handle);
	}
	Framebuffer& Framebuffer::operator=(Framebuffer&& rhs)noexcept {
		if(_fb_handle)
			glDeleteFramebuffers(1, &_fb_handle);

		if(_db_handle)
			glDeleteRenderbuffers(1, &_db_handle);

		Texture::operator=(std::move(rhs));

		_fb_handle = rhs._fb_handle;
		_db_handle = rhs._db_handle;

		rhs._fb_handle = 0;
		rhs._db_handle = 0;

		return *this;
	}

	void Framebuffer::set_viewport() {
		glViewport(0,0, width(), height());
	}

	void Framebuffer::clear(glm::vec3 color) {
		glClearColor(color.r, color.g, color.b, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | (_db_handle ? GL_DEPTH_BUFFER_BIT : 0));
	}

	void Framebuffer::bind_target() {
		glBindFramebuffer(GL_FRAMEBUFFER, _fb_handle);
		glViewport(0,0, width(), height());
	}
	void Framebuffer::unbind_target() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Framebuffer_binder::Framebuffer_binder(Framebuffer& fb) : fb(fb) {
		glGetIntegerv(GL_VIEWPORT, old_viewport);
		fb.bind_target();
	}
	Framebuffer_binder::~Framebuffer_binder()noexcept {
		fb.unbind_target();
		glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
	}

} /* namespace renderer */
}
