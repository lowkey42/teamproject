#define GLM_SWIZZLE

#include "texture_batch.hpp"

#include "command_queue.hpp"

namespace mo {
namespace renderer {

	using namespace glm;

	namespace {
		Vertex_layout tex_layout {
			Vertex_layout::Mode::triangles,
			vertex("position",  &Texture_Vertex::position),
			vertex("uv",        &Texture_Vertex::uv)
		};

		std::unique_ptr<Shader_program> tex_shader;
		const Texture_Vertex single_tex_vert[] {
			Texture_Vertex{{-0.5f,-0.5f}, {0,0}, nullptr},
			Texture_Vertex{{-0.5f,+0.5f}, {0,1}, nullptr},
			Texture_Vertex{{+0.5f,+0.5f}, {1,1}, nullptr},

			Texture_Vertex{{+0.5f,+0.5f}, {1,1}, nullptr},
			Texture_Vertex{{-0.5f,-0.5f}, {0,0}, nullptr},
			Texture_Vertex{{+0.5f,-0.5f}, {1,0}, nullptr}
		};
	}

	Texture_Vertex::Texture_Vertex(glm::vec2 pos, glm::vec2 uv_coords,
	                             const renderer::Texture* texture)
	    : position(pos), uv(uv_coords), tex(texture) {
	}


	void init_texture_renderer(asset::Asset_manager& asset_manager) {
		tex_shader = std::make_unique<Shader_program>();
		tex_shader->attach_shader(asset_manager.load<Shader>("vert_shader:simple"_aid))
		           .attach_shader(asset_manager.load<Shader>("frag_shader:simple"_aid))
		           .bind_all_attribute_locations(tex_layout)
		           .build()
		           .uniforms(make_uniform_map(
		               "texture", int(Texture_unit::color),
		               "model", glm::mat4(),
		               "layer", 0,
		               "clip", glm::vec4{0,0,1,1},
		               "color", glm::vec4{1,1,1,1}
		           ));
	}

	Texture_batch::Texture_batch(std::size_t expected_size) {
		_vertices.reserve(expected_size*4);
		_objects.reserve(expected_size*0.25f);
	}

	void Texture_batch::insert(const Texture& texture, glm::vec2 pos, glm::vec2 size) {
		auto scale = vec2 {
			size.x,
			size.y
		};

		auto transform = [&](vec2 p) {
			return pos + p*scale;
		};

		auto tex_clip = texture.clip_rect();
		auto sprite_clip = glm::vec4{0,0,1,1};

		// rescale uv to texture clip_rect
		sprite_clip.xz() *= (tex_clip.z - tex_clip.x);
		sprite_clip.yw() *= (tex_clip.w - tex_clip.y);
		// move uv by clip_rect offset
		sprite_clip.xz() += tex_clip.x;
		sprite_clip.yw() += tex_clip.y;

		auto uv_size = sprite_clip.zw() - sprite_clip.xy();

		for(auto& vert : single_tex_vert) {
			auto uv = sprite_clip.xy() + uv_size * vert.uv;
			_vertices.emplace_back(transform(vert.position), uv, &texture);
		}
	}

	void Texture_batch::flush(Command_queue& queue) {
		_draw(queue);
		_vertices.clear();
		_free_obj = 0;
	}

	void Texture_batch::_draw(Command_queue& queue) {
		// partition _vertices by texture
		std::stable_sort(_vertices.begin(), _vertices.end());

		// draw one batch for each partition
		auto last = _vertices.begin();
		for(auto current = _vertices.begin(); current!=_vertices.end(); ++current) {
			if(current->tex != last->tex) {
				queue.push_back(_draw_part(last, current));
				last = current;
			}
		}

		if(last!=_vertices.end())
			queue.push_back(_draw_part(last, _vertices.end()));
	}

	auto Texture_batch::_draw_part(Vertex_citer begin, Vertex_citer end) -> Command {
		INVARIANT(begin!=end && begin->tex, "Invalid iterators");

		auto obj_idx = _free_obj++;

		if(obj_idx>=_objects.size()) {
			_objects.emplace_back(tex_layout, create_buffer<Texture_Vertex>(begin, end, true));
		} else {
			_objects.at(obj_idx).buffer().set<Texture_Vertex>(begin, end);
		}

		auto cmd = create_command()
		        .shader(*tex_shader)
		        .require_not(Gl_option::depth_test)
		        .require_not(Gl_option::depth_write)
		        .texture(Texture_unit::color, *begin->tex)
		        .object(_objects.at(obj_idx));

		return cmd;
	}

}
}
