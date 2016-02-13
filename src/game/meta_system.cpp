#include "meta_system.hpp"

#include <core/renderer/command_queue.hpp>
#include <core/renderer/uniform_map.hpp>


namespace mo {

	using namespace unit_literals;

	namespace {
		constexpr Distance max_entity_size = 20_m;

		constexpr auto global_uniforms = 4+sys::light::light_uniforms;
		constexpr auto global_uniforms_size = 4*(4*4)+sys::light::light_uniforms_size;
		constexpr auto global_uniforms_avg_size = (int)std::ceil(global_uniforms_size/global_uniforms);

		using Global_uniform_map = renderer::Uniform_map<global_uniforms,
		                                                 global_uniforms_avg_size*sizeof(float)>;
	}

	Meta_system::Meta_system(Engine& engine)
	    : entity_manager(engine.assets()),
	      scene_graph(entity_manager, max_entity_size),
	      lights(engine.bus(), entity_manager, scene_graph),
	      renderer(engine.bus(), entity_manager, scene_graph, engine.assets()) {

		_render_queue.shared_uniforms(std::make_unique<Global_uniform_map>());
	}

	Meta_system::~Meta_system() {
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

	void Meta_system::draw(const renderer::Camera& cam)const {
		_render_queue.shared_uniforms().emplace("VP", cam.vp());

		lights.draw(_render_queue, cam);
		renderer.draw(_render_queue, cam);

		_render_queue.flush();
	}

}
