#include "meta_system.hpp"

#include <core/renderer/command_queue.hpp>
#include <core/renderer/uniform_map.hpp>


namespace mo {

	using namespace unit_literals;

	constexpr Distance max_entity_size = 20_m;

	Meta_system::Meta_system(Engine& engine)
	    : entity_manager(engine.assets()),
	      scene_graph(entity_manager, max_entity_size),
	      renderer(engine.bus(), entity_manager, scene_graph, engine.assets()) {

		_render_queue.shared_uniforms(std::make_unique<renderer::Uniform_map<4,sizeof(float)*4*4>>());
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

		renderer.draw(_render_queue, cam);

		_render_queue.flush();
	}

}
