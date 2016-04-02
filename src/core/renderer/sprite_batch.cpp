#define GLM_SWIZZLE

#include "sprite_batch.hpp"

#include "command_queue.hpp"

namespace lux {
namespace renderer {

	using namespace glm;

	namespace {
		std::unique_ptr<Shader_program> sprite_shader;
		const Sprite_vertex single_sprite_vert[] {
			Sprite_vertex{{-0.5f,-0.5f, 0.f}, {0,1}, 0,0, nullptr},
			Sprite_vertex{{-0.5f,+0.5f, 0.f}, {0,0}, 0,0, nullptr},
			Sprite_vertex{{+0.5f,+0.5f, 0.f}, {1,0}, 0,0, nullptr},

			Sprite_vertex{{+0.5f,+0.5f, 0.f}, {1,0}, 0,0, nullptr},
			Sprite_vertex{{-0.5f,-0.5f, 0.f}, {0,1}, 0,0, nullptr},
			Sprite_vertex{{+0.5f,-0.5f, 0.f}, {1,1}, 0,0, nullptr}
		};
	}

	Vertex_layout sprite_layout {
		Vertex_layout::Mode::triangles,
		vertex("position",  &Sprite_vertex::position),
		vertex("uv",        &Sprite_vertex::uv),
		vertex("rotation",  &Sprite_vertex::rotation),
		vertex("shadow_resistence", &Sprite_vertex::shadow_resistence)
	};

	Sprite::Sprite(glm::vec3 position, Angle rotation, glm::vec2 size,
	               glm::vec4 uv, float shadow_resistence,
	               const renderer::Material& material)noexcept
	    : position(position), rotation(rotation), size(size),
	      uv(uv), shadow_resistence(shadow_resistence), material(&material) {
	}

	Sprite_vertex::Sprite_vertex(glm::vec3 pos, glm::vec2 uv_coords,
	                             float rotation, float shadow_resistence,
	                             const renderer::Material* material)
	    : position(pos), uv(uv_coords), rotation(rotation),
	      shadow_resistence(shadow_resistence), material(material) {
	}


	void init_sprite_renderer(asset::Asset_manager& asset_manager) {
		sprite_shader = std::make_unique<Shader_program>();
		sprite_shader->attach_shader(asset_manager.load<Shader>("vert_shader:sprite"_aid))
		              .attach_shader(asset_manager.load<Shader>("frag_shader:sprite"_aid))
		              .bind_all_attribute_locations(sprite_layout)
		              .build()
		              .uniforms(make_uniform_map(
		                  "albedo_tex", int(Texture_unit::color),
		                  "normal_tex", int(Texture_unit::normal),
		                  "emission_tex", int(Texture_unit::emission),
		                  "roughness_tex", int(Texture_unit::roughness),
		                  "metallic_tex", int(Texture_unit::metallic),
		                  "height_tex", int(Texture_unit::height),
		                  "shadowmap_1_tex", int(Texture_unit::shadowmap_1),
		                  "shadowmap_2_tex", int(Texture_unit::shadowmap_2),
		                  "environment_tex", int(Texture_unit::environment),
		                  "last_frame_tex", int(Texture_unit::last_frame)
		              ));
	}

	Sprite_batch::Sprite_batch(std::size_t expected_size)
	    : Sprite_batch(*sprite_shader, expected_size) {
	}
	Sprite_batch::Sprite_batch(Shader_program& shader, std::size_t expected_size)
	    : _shader(shader) {

		_vertices.reserve(expected_size*4);
		_objects.reserve(expected_size*0.25f);
	}

	void Sprite_batch::insert(const Sprite& sprite) {
		auto scale = vec3 {
			sprite.size.x,
			sprite.size.y,
			1.f
		};

		auto transform = [&](vec3 p) {
			return sprite.position + rotate(p, sprite.rotation, vec3{0,0,1})*scale;
		};

		auto tex_clip = sprite.material->albedo().clip_rect();
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
			_vertices.emplace_back(transform(vert.position), uv,
			                       sprite.rotation.value(), sprite.shadow_resistence, sprite.material);
		}
	}
	void Sprite_batch::insert(glm::vec3 position,
	                          const std::vector<Sprite_vertex>& vertices) {
		_vertices.reserve(_vertices.size() + vertices.size());

		for(auto& v : vertices) {
			_vertices.emplace(v.position + position,
			                  v.uv, v.rotation, v.shadow_resistence, v.material);
		}
	}

	void Sprite_batch::flush(Command_queue& queue) {
		_draw(queue);
		_vertices.clear();
		_free_obj = 0;
	}

	void Sprite_batch::_draw(Command_queue& queue) {
		// partition _vertices by texture
		std::stable_sort(_vertices.begin(), _vertices.end());

		// draw one batch for each partition
		auto last = _vertices.begin();
		for(auto current = _vertices.begin(); current!=_vertices.end(); ++current) {
			if(current->material != last->material) {
				queue.push_back(_draw_part(last, current));
				last = current;
			}
		}

		if(last!=_vertices.end())
			queue.push_back(_draw_part(last, _vertices.end()));
	}

	auto Sprite_batch::_draw_part(Vertex_citer begin, Vertex_citer end) -> Command {
		INVARIANT(begin!=end && begin->material, "Invalid iterators");

		auto obj_idx = _free_obj++;

		if(obj_idx>=_objects.size()) {
			_objects.emplace_back(sprite_layout, create_buffer<Sprite_vertex>(begin, end, true));
		} else {
			_objects.at(obj_idx).buffer().set<Sprite_vertex>(begin, end);
		}

		auto cmd = create_command()
		        .shader(_shader)
		        .object(_objects.at(obj_idx));

		begin->material->set_textures(cmd);

		cmd.uniforms().emplace("model", glm::mat4());

		return cmd;
	}

}
}
