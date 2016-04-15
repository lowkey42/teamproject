#include "meta_system.hpp"

#include <core/renderer/graphics_ctx.hpp>
#include <core/renderer/command_queue.hpp>
#include <core/renderer/uniform_map.hpp>
#include <core/renderer/texture.hpp>
#include <core/renderer/texture_batch.hpp>
#include <core/renderer/primitives.hpp>


namespace lux {

	using namespace renderer;
	using namespace unit_literals;

	namespace {

		constexpr auto global_uniforms = 4+sys::light::light_uniforms;
		constexpr auto global_uniforms_size = 4*(4*4)+sys::light::light_uniforms_size;
		constexpr auto global_uniforms_avg_size = (int)(global_uniforms_size/global_uniforms + 0.5f);

		using Global_uniform_map = renderer::Uniform_map<global_uniforms,
		                                                 global_uniforms_avg_size*sizeof(float)>;

		auto create_framebuffer(Engine& engine) {
			return Framebuffer{
				engine.graphics_ctx().win_width(),
				engine.graphics_ctx().win_height(),
				true, true};
		}
	}

	Meta_system::Meta_system(Engine& engine)
	    : entity_manager(engine.assets()),
	      scene_graph(entity_manager),
	      lights(engine.bus(), entity_manager, engine.assets()),
	      renderer(engine.bus(), entity_manager, engine.assets()),

	      _engine(engine),
	      _canvas{create_framebuffer(engine), create_framebuffer(engine)},
	      _skybox(engine.assets(), "tex_cube:default_env"_aid /*TODO: should be loaded*/) {

		_render_queue.shared_uniforms(std::make_unique<Global_uniform_map>());

		_post_shader.attach_shader(engine.assets().load<Shader>("vert_shader:post"_aid))
		            .attach_shader(engine.assets().load<Shader>("frag_shader:post"_aid))
		            .bind_all_attribute_locations(simple_vertex_layout)
		            .build()
		            .uniforms(make_uniform_map(
		                "texture", int(Texture_unit::temporary), "gamma", 2.2f, "exposure", 1.0f
		            ));
	}

	Meta_system::~Meta_system() {
	}


	auto Meta_system::load_level(const std::string& id) -> Level_data {
		auto level_meta_data = ::lux::load_level(_engine, entity_manager, id);
		_skybox.texture(_engine.assets().load<Texture>({"tex_cube"_strid, level_meta_data.environment_id}));
		// TODO: setup the rest of the subsystems with level_meta_data

		return level_meta_data;
	}

	void Meta_system::update(Time dt, Update mask) {
		update(dt, static_cast<Update_mask>(mask));
	}

	void Meta_system::update(Time dt, Update_mask mask) {
		entity_manager.process_queued_actions();

		if(mask & Update::movements) {
			scene_graph.update(dt);
		}

		if(mask & Update::animations) {
			renderer.update(dt);
		}
	}

	void Meta_system::draw(const renderer::Camera& cam) {
		auto uniforms = _render_queue.shared_uniforms();

		renderer.draw_shadowcaster(lights.shadowcaster_batch(), cam);
		lights.prepare_draw(_render_queue, cam);

		uniforms->emplace("vp", cam.vp());
		uniforms->emplace("eye", cam.eye_position());

		renderer.draw(_render_queue, cam);

		_skybox.draw(_render_queue);

		{
			auto fbo_cleanup = Framebuffer_binder{_active_canvas()};
			_active_canvas().clear();

			_render_queue.flush();
		}

		_post_shader.bind()
		        .set_uniform("exposure", 1.0f); // TODO: calc real exposure
		renderer::draw_fullscreen_quad(_active_canvas());

		_canvas_first_active = !_canvas_first_active;
	}

}
