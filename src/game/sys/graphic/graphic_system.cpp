#include "graphic_system.hpp"

#include "../physics/transform_comp.hpp"

#include <core/units.hpp>
#include <core/renderer/command_queue.hpp>


namespace lux {
namespace sys {
namespace graphic {

	using namespace renderer;
	using namespace unit_literals;

	namespace {
		constexpr auto background_boundary = -10.f;

		auto build_background_shader(asset::Asset_manager& asset_manager) -> Shader_program {
			Shader_program prog;
			prog.attach_shader(asset_manager.load<Shader>("vert_shader:sprite"_aid))
			    .attach_shader(asset_manager.load<Shader>("frag_shader:sprite_bg"_aid))
			    .bind_all_attribute_locations(sprite_layout)
			    .build()
			    .uniforms(make_uniform_map(
			                  "albedo_tex", int(Texture_unit::color),
			                  "normal_tex", int(Texture_unit::normal),
			                  "material_tex", int(Texture_unit::material),
			                  "environment_tex", int(Texture_unit::environment)
			    ));

			return prog;
		}

		auto flip(glm::vec4 uv_clip, bool vert, bool horiz) {
			return glm::vec4 {
				horiz ? uv_clip.z : uv_clip.x,
				vert  ? uv_clip.w : uv_clip.y,
				horiz ? uv_clip.x : uv_clip.z,
				vert  ? uv_clip.y : uv_clip.w
			};
		}
	}

	Graphic_system::Graphic_system(
	        util::Message_bus& bus,
	        ecs::Entity_manager& entity_manager,
	        asset::Asset_manager& asset_manager)
	    : _background_shader(build_background_shader(asset_manager)),
	      _mailbox(bus),
	      _sprites(entity_manager.list<Sprite_comp>()),
	      _anim_sprites(entity_manager.list<Anim_sprite_comp>()),
	      _terrains(entity_manager.list<Terrain_comp>()),
		  _particles(entity_manager.list<Particle_comp>()),
		  _particle_renderer(asset_manager),
	      _sprite_batch(512),
	      _sprite_batch_bg(_background_shader, 256)
	{
		entity_manager.register_component_type<Sprite_comp>();
		entity_manager.register_component_type<Anim_sprite_comp>();
		entity_manager.register_component_type<Terrain_comp>();
		entity_manager.register_component_type<Terrain_data_comp>();

		_mailbox.subscribe_to<16, 128>([&](const State_change& e) {
			this->_on_state_change(e);
		});
	}

	void Graphic_system::draw(renderer::Command_queue& queue, const renderer::Camera& camera)const {
		for(Sprite_comp& sprite : _sprites) {
			auto& trans = sprite.owner().get<physics::Transform_comp>().get_or_throw();

			auto position = remove_units(trans.position());
			auto sprite_data = renderer::Sprite{
			                   position, trans.rotation(),
			                   sprite._size*trans.scale(),
			                   flip(glm::vec4{0,0,1,1}, trans.flip_vertical(), trans.flip_horizontal()),
			                   sprite._shadowcaster ? 1.0f : 1.f-sprite._shadow_receiver,
			                   sprite._decals_intensity, *sprite._material};

			sprite_data.hue_change = {
			    sprite._hue_change_target / 360_deg,
			    sprite._hue_change_replacement / 360_deg
			};

			if(position.z<background_boundary) {
				_sprite_batch_bg.insert(sprite_data);
			} else {
				_sprite_batch.insert(sprite_data);
			}
		}

		for(Anim_sprite_comp& sprite : _anim_sprites) {
			auto& trans = sprite.owner().get<physics::Transform_comp>().get_or_throw();

			auto position = remove_units(trans.position());
			auto sprite_data = renderer::Sprite{
			                   position, trans.rotation(),
			                   sprite._size*trans.scale(),
			                   flip(sprite.state().uv_rect(), trans.flip_vertical(), trans.flip_horizontal()),
			                   sprite._shadowcaster ? 1.0f : 1.f-sprite._shadow_receiver,
			                   sprite._decals_intensity, sprite.state().material()};

			sprite_data.hue_change = {
			    sprite._hue_change_target / 360_deg,
			    sprite._hue_change_replacement / 360_deg
			};

			if(position.z<background_boundary) {
				_sprite_batch_bg.insert(sprite_data);
			} else {
				_sprite_batch.insert(sprite_data);
			}
		}

		for(Terrain_comp& terrain : _terrains) {
			auto& trans = terrain.owner().get<physics::Transform_comp>().get_or_throw();
			auto position = remove_units(trans.position());

			if(position.z<background_boundary) {
				terrain._smart_texture.draw(position, _sprite_batch_bg);
			} else {
				terrain._smart_texture.draw(position, _sprite_batch);
			}
		}

		_sprite_batch.flush(queue);
		_sprite_batch_bg.flush(queue);

		_particle_renderer.draw(queue);
	}
	void Graphic_system::draw_shadowcaster(renderer::Sprite_batch& batch,
	                                       const renderer::Camera&)const {
		for(Sprite_comp& sprite : _sprites) {
			auto& trans = sprite.owner().get<physics::Transform_comp>().get_or_throw();

			auto position = remove_units(trans.position());

			if(sprite._shadowcaster && std::abs(position.z) < 1.0f) {
				batch.insert(renderer::Sprite{position, trans.rotation(),
				             sprite._size*trans.scale(),
				             flip(glm::vec4{0,0,1,1}, trans.flip_vertical(), trans.flip_horizontal()),
				             sprite._shadowcaster ? 1.0f : 0.0f,
				             sprite._decals_intensity, *sprite._material});
			}
		}

		for(Anim_sprite_comp& sprite : _anim_sprites) {
			auto& trans = sprite.owner().get<physics::Transform_comp>().get_or_throw();

			auto position = remove_units(trans.position());

			if(sprite._shadowcaster && std::abs(position.z) < 1.0f) {
				batch.insert(renderer::Sprite{position, trans.rotation(),
				             sprite._size*trans.scale(),
				             flip(sprite.state().uv_rect(), trans.flip_vertical(), trans.flip_horizontal()),
				             sprite._shadowcaster ? 1.0f : 0.0f,
				             sprite._decals_intensity, sprite.state().material()});
			}
		}

		for(Terrain_comp& terrain : _terrains) {
			auto& trans = terrain.owner().get<physics::Transform_comp>().get_or_throw();
			auto position = remove_units(trans.position());

			if(terrain._smart_texture.shadowcaster() && std::abs(position.z) < 1.0f) {
				terrain._smart_texture.draw(position, batch);
			}
		}
	}

	void Graphic_system::update(Time dt) {
		for(Anim_sprite_comp& sprite : _anim_sprites) {
			sprite.state().update(dt, _mailbox.bus());
		}

		for(Particle_comp& particle : _particles) {
			for(auto& id : particle._add_queue) {
				if(id!=""_strid) {
					auto existing = std::find_if(particle._emitters.begin(), particle._emitters.end(),
					                            [&](auto& e){return e && e->type()==id;});

					if(existing==particle._emitters.end()) {
						auto empty = std::find(particle._emitters.begin(), particle._emitters.end(),
						                       renderer::Particle_emitter_ptr{});
						if (empty == particle._emitters.end())
							empty = particle._emitters.begin();

						*empty = _particle_renderer.create_emiter(id);
					}
					id = ""_strid;
				}
			}

			auto& transform = particle.owner().get<physics::Transform_comp>().get_or_throw();
			auto position = remove_units(transform.position()) + particle._offset;
			for(auto& e : particle._emitters) {
				if(e) {
					e->hue_change_out(particle._hue_change);
					e->position(position);
					e->direction(glm::vec3(0,0,transform.rotation().value()));
					e->scale(transform.scale());
				}
			}
		}

		_particle_renderer.update(dt);
	}

	void Graphic_system::_on_state_change(const State_change& s) {
	}

}
}
}
