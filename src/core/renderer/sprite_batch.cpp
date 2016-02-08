#define GLM_SWIZZLE

#include "sprite_batch.hpp"

#include "command_queue.hpp"

namespace mo {
namespace renderer {

	using namespace glm;

	namespace {
		Vertex_layout sprite_layout {
			Vertex_layout::Mode::triangles,
			vertex("position",  &Sprite_vertex::position),
			vertex("uv",        &Sprite_vertex::uv)
		};

		std::unique_ptr<Shader_program> sprite_shader;
		const Sprite_vertex single_sprite_vert[] {
			Sprite_vertex{{-0.5f,-0.5f, 0.f}, {0,0}, nullptr},
			Sprite_vertex{{-0.5f,+0.5f, 0.f}, {0,1}, nullptr},
			Sprite_vertex{{+0.5f,+0.5f, 0.f}, {1,1}, nullptr},

			Sprite_vertex{{+0.5f,+0.5f, 0.f}, {1,1}, nullptr},
			Sprite_vertex{{-0.5f,-0.5f, 0.f}, {0,0}, nullptr},
			Sprite_vertex{{+0.5f,-0.5f, 0.f}, {1,0}, nullptr}
		};
	}

	Sprite::Sprite(Position position, float layer, Angle rotation, Position size,
	               glm::vec4 uv, const renderer::Texture& texture)noexcept
	    : position(position), layer(layer), rotation(rotation), size(size),
	      uv(uv), texture(&texture) {
	}

	Sprite_vertex::Sprite_vertex(glm::vec3 pos, glm::vec2 uv_coords,
	                             const renderer::Texture* texture)
	    : position(pos), uv(uv_coords), texture(texture) {
	}


	void init_sprite_renderer(asset::Asset_manager& asset_manager) {
		sprite_shader = std::make_unique<Shader_program>();
		sprite_shader->attach_shader(asset_manager.load<Shader>("vert_shader:sprite"_aid))
		              .attach_shader(asset_manager.load<Shader>("frag_shader:sprite"_aid))
		              .bind_all_attribute_locations(sprite_layout)
		              .build()
		              .uniforms(make_uniform_map(
		                  "texture", 0
		              ));
	}

	Sprite_batch::Sprite_batch(std::size_t expected_size) {
		_vertices.reserve(expected_size*4);
		_objects.reserve(expected_size*0.25f);
	}

	void Sprite_batch::insert(const Sprite& sprite) {
		auto offset = vec3 {
			sprite.position.x.value(),
			sprite.position.y.value(),
			sprite.layer
		};
		auto scale = vec3 {
			sprite.size.x.value(),
			sprite.size.y.value(),
			1.f
		};

		auto transform = [&](vec3 p) {
			return offset + rotate(p, sprite.rotation, vec3{0,0,1})*scale;
		};

		auto tex_clip = sprite.texture->clip_rect();
		auto sprite_clip = sprite.uv;

		// rescale uv to texture clip_rect
		sprite_clip.xz() *= (tex_clip.z - tex_clip.x);
		sprite_clip.yw() *= (tex_clip.w - tex_clip.y);
		// move uv by clip_rect offset
		sprite_clip.xz() += tex_clip.x;
		sprite_clip.yw() += tex_clip.y;

		auto uv_size = sprite_clip.zw() - sprite_clip.xy();

		for(auto& vert : single_sprite_vert) {
			auto uv = sprite_clip.xy() + uv_size * vert.uv;
			_vertices.emplace_back(transform(vert.position), uv, sprite.texture);
		}
	}

	void Sprite_batch::flush(Command_queue& queue) {
		_draw(queue);
		_vertices.clear();
	}

	void Sprite_batch::_draw(Command_queue& queue) {
		// partition _vertices by texture
		std::stable_sort(_vertices.begin(), _vertices.end());

		// draw one batch for each partition
		auto last = _vertices.begin();
		for(auto current = _vertices.begin(); current!=_vertices.end(); ++current) {
			if(current->texture != last->texture) {
				queue.push_back(_draw_part(last, current));
				last = current;
			}
		}

		if(last!=_vertices.end())
			queue.push_back(_draw_part(last, _vertices.end()));
	}

	auto Sprite_batch::_draw_part(Vertex_citer begin, Vertex_citer end) -> Command {
		INVARIANT(begin!=end && begin->texture, "Invalid iterators");

		auto obj_idx = _free_obj++;

		if(obj_idx>=_objects.size()) {
			_objects.emplace_back(sprite_layout, create_buffer<Sprite_vertex>(begin, end, true));
		} else {
			_objects.at(obj_idx).buffer().set<Sprite_vertex>(begin, end);
		}

		auto cmd = create_command()
		        .texture(Texture_unit::color, *begin->texture)
		        .shader(*sprite_shader)
		        .object(_objects.at(obj_idx));

		cmd.uniforms().emplace("model", glm::mat4());

		return cmd;
	}

}
}
