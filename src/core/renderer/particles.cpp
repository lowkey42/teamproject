#include "particles.hpp"

#include "texture.hpp"
#include "vertex_object.hpp"

#include <vector>


namespace lux {
namespace renderer {

	sf2_structDef(Float_range,
		min, max
	)

	sf2_structDef(Attractor_plane, normal, force)
	sf2_structDef(Attractor_point, point, force)

	sf2_structDef(Particle_type,
		id,
		physics_simulation,
		mass,
		texture,

		animation_frames,
		fps,

		initial_alpha,
		final_alpha,
		initial_opacity,
		final_opacity,

		initial_size,
		final_size,
		rotation,
		lifetime,

		emision_rate,
		max_particle_count,

		spawn_x, spawn_y, spawn_z,

		initial_pitch,
		initial_yaw,
		initial_roll,

		speed_pitch,
		speed_yaw,
		speed_roll,

		initial_speed,
		final_speed,

		attractor_plane,
		attractor_point
	)

	namespace {
		struct Particle_base_vertex {
			glm::vec2 xy;
			glm::vec2 uv;
		};

		struct Particle_draw {
			float ttl; //< required for deletion

			glm::vec3 position;
			float rotation;
			float frames;
			float current_frame;
			float size;

			float alpha;
			float opacity;
		};
		Vertex_layout simple_particle_vertex_layout {
			Vertex_layout::Mode::triangle_strip,
			vertex("xy",            &Base::vertex::xy,             0,0)
			vertex("uv",            &Base::vertex::uv,             0,0)
			vertex("position",      &Particle_draw::position,      1,1),
			vertex("rotation",      &Particle_draw::rotation,      1,1),
			vertex("frames",        &Particle_draw::frames,        1,1),
			vertex("current_frame", &Particle_draw::current_frame, 1,1),
			vertex("size",          &Particle_draw::size,          1,1),
			vertex("alpha",         &Particle_draw::alpha,         1,1),
			vertex("opacity",       &Particle_draw::opacity,       1,1),
		};
		std::vector<Particle_base_vertex> simple_particle_vertices {
			{{-.5,-.5}, {0,1}},
			{{ .5,-.5}, {1,1}},
			{{-.5, .5}, {0,0}},
			{{ .5, .5}, {1,0}}
		};

		struct Particle_sim {
			Time ttl;
			Time lifetime;

			float initial_alpha;
			float final_alpha;
			float initial_opacity; // TODO synonym/spelling?
			float final_opacity;

			float initial_size;
			float final_size;

			glm::quat direction;
			float speed;
		};
	}

	class Particle_emiter {
		public:
			Particle_emiter() = default;
			virtual ~Particle_emiter() = default;

			void position(glm::vec3 position) {
				_position = position;
			}
			void direction(glm::vec3 eular_angles) {
				_direction = eular_angles;
			}

			virtual auto texture()const noexcept -> const Texture* = 0;
			virtual void update(Time dt) = 0;
			virtual void draw(Command& cmd)const = 0;
			virtual void disable() {_active = false;}
			virtual bool dead()const noexcept {return !_active;}

		protected:
			glm::vec3 _position;
			glm::vec3 _direction;
			bool _active = true;
	};

	class Simple_particle_emiter : public Particle_emiter {
		public:
			Simple_particle_emiter(asset::Asset_manager&, assets, Particle_type& type)
			    : _type(type),
			      _obj(simple_particle_vertex_layout,
			           create_buffer(simple_particle_vertices),
			           create_dynamic_buffer<Particle_draw>(type.max_particle_count)) {

				_texture = assets.load<Texture>(asset::AID{type.texture});

				_particles_draw.reserve(type.max_particle_count);
				_particles_sim.reserve(type.max_particle_count);
			}

			auto texture()const noexcept -> const Texture* override {return &*_texture;}

			void update(Time dt) override {
				INVARIANT(_particles_draw.size()==_particles_sim.size(), "Size mismatch in particle sim/draw buffer.");

				_dt_acc += dt;
				_to_spawn = std::round(_type.emision_rate * _dt_acc.value());
				_dt_acc-=Time(to_spawn/_spawn_rate);

				if(_active) {
					_reap(dt);
					_spawn();
				}

				_simulation(dt);
			}

			void draw(Command& cmd)const override {
				_obj.buffer(1).set(_particles_draw);
				return cmd.object(_obj)
				          .texture(Texture_unit::color, *_texture);
			}

			bool dead()const noexcept override {return !_active && _particles_draw.empty();}

		private:
			Particle_type& _type;

			Texture_ptr _texture;
			Object _obj;

			std::vector<Particle_draw> _particles_draw;
			std::vector<Particle_sim> _particles_sim;

			int_fast16_t _to_spawn;
			Time _dt_acc{0};


			void _reap(Time dt) {
				auto to_spawn = _to_spawn;
				auto new_end = int_fast32_t(_particles_sim.size());

				for(auto i=0; i<new_end, i++) {
					auto& curr = _particles_sim[i];
					// TODO: optimize read after store?
					curr.ttl-=dt;
					auto dead = curr.ttl<=0_s;

					if(dead) {
						if(to_spawn>0) {
							_spawn_particle_at(i);
							to_spawn--;
						} else {
							std::swap(curr, _particles_sim[new_end-1]);
							std::swap(curr, _particles_draw[new_end-1]);
							i--;
							new_end--;
						}
					}
				}

				if(new_end<_particles_sim.size()) {
					_particles_sim.erase(_particles_sim.begin()+new_end,
					                     _particles_sim.end());
					_particles_draw.erase(_particles_draw.begin()+new_end,
					                     _particles_draw.end());
				}

				_to_spawn = to_spawn;
			}

			void _spawn() {
				_to_spawn = glm::min(_to_spawn, _type.max_particle_count - int(_particles_sim.size()));

				for(auto i : util::numeric_range(to_spawn)) {
					(void)i;

					_particles_sim.emplace_back();
					_particles_draw.emplace_back();
					_spawn_particle_at(_particles_sim.size()-1);
				}
			}
			void _spawn_particle_at(int_fast32_t idx) {
				// TODO: create particle
			}

			void _simulation(Time dt) {
				for(std::size_t i=0; i<_particles_sim.size(); i++) {
					auto& curr_sim = _particles_sim[i];
					auto& curr_draw = _particles_draw[i];

					auto a = 1.f - curr_sim.ttl/curr_sim.lifetime;

					curr_draw.alpha = glm::mix(curr_sim.initial_alpha, curr_sim.final_alpha, a);
					curr_draw.opacity = glm::mix(curr_sim.initial_opacity, curr_sim.final_opacity, a);
					curr_draw.size = glm::mix(curr_sim.initial_size, curr_sim.final_size, a);

					if(_type.fps<0) {
						curr_draw.current_frame = curr_draw.frames*a;
					} else {
						auto next_frame = curr_draw.current_frame + _type.fps*dt.value();
						curr_draw.current_frame = std::fmod(next_frame, curr_draw.frames);
					}

					// TODO: update position and rotation
				}
			}
	};


	void set_position(Particle_emiter& e, glm::vec3 position) {
		e.position(position);
	}

	void set_direction(Particle_emiter& e, glm::vec3 eular_angles) {
		e.direction(eular_angles);
	}

	Particle_renderer::Particle_renderer(Engine& e) {
		_simple_shader.attach_shader(asset_manager.load<Shader>("vert_shader:particle"_aid))
		              .attach_shader(asset_manager.load<Shader>("frag_shader:particle"_aid))
		              .bind_all_attribute_locations(simple_particle_vertices)
		              .build()
		              .uniforms(make_uniform_map(
		                  "albedo_tex", int(Texture_unit::color),
		                  "shadowmaps_tex", int(Texture_unit::shadowmaps),
		                  "environment_tex", int(Texture_unit::environment),
		                  "last_frame_tex", int(Texture_unit::last_frame),
		              ));
	}

	auto Particle_renderer::create_emiter(Particle_type_id id) -> Particle_emiter_ptr {
		auto iter = _types.find(id);
		if(iter==_types.end()) {
			WARN("Created emiter of unknown type '"<<id.str()<<"'");
			iter = _types.begin();
		}

		auto emiter = Particle_emiter_ptr{};

		if(iter->second->physics_simulation) {
			FAIL("NOT IMPLEMENTED, YET!");
		} else {
			emiter = std::make_shared<Particle_emiter>(*iter->second);
		}

		_emiters.emplace_back(emiter);

		std::sort(_emiters.begin(), _emiters.end(), [](auto& lhs, auto& rhs){
			return lhs.texture() < rhs.texture();
		})

		return emiter;
	}

	void Particle_renderer::update(Time dt) {
		for(auto& e : _emiters) {
			if(e->use_count()<=1) {
				e->disable();
			}
			e->update(dt);
		}

		_emiters.erase(std::remove_if(_emiters.begin(),_emiters.end(), [](auto& e){return e->dead();}), _emiters.end());
	}
	void Particle_renderer::draw(Command_queue& queue)const {
		for(auto& e : _emiters) {
			queue.push_back(e->draw(
			                    create_command().shader(_simple_shader)
			));

		}
	}

}
}
