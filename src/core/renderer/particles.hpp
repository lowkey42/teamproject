/** 3D particle system *******************************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "command_queue.hpp"
#include "shader.hpp"

#include "../units.hpp"


namespace lux {
	class Engine;

namespace renderer {

	using Particle_type_id = util::Str_id;

	struct Float_range {
		float min;
		float max;
	};

	struct Attractor_plane {
		glm::vec3 normal;
		float force = 0.f;
	};
	struct Attractor_point {
		glm::vec3 point;
		float force = 0.f;
	};

	struct Particle_type {
		Particle_type_id id;
		bool physics_simulation = false;
		float mass = 1.f;
		std::string texture;

		int animation_frames = 1;
		float fps = 1.f;

		Float_range initial_alpha {1.f,1.f};
		Float_range final_alpha {1.f,1.f};
		Float_range initial_opacity {1.f,1.f};
		Float_range final_opacity {1.f,1.f};

		Float_range initial_size {0.2f,0.2f};
		Float_range final_size {0.2f,0.2f};
		Float_range rotation {0.f,0.f};
		Float_range lifetime {1.f,1.f};

		Float_range emision_rate {100.f, 100.f};
		int max_particle_count = 1000;

		Float_range spawn_x {-1.f,1.f};
		Float_range spawn_y {-1.f,1.f};
		Float_range spawn_z {-1.f,1.f};

		Float_range initial_pitch {0.f,0.f};
		Float_range initial_yaw {0.f,0.f};
		Float_range initial_roll {0.f,0.f};

		Float_range speed_pitch {0.f,0.f};
		Float_range speed_yaw {0.f,0.f};
		Float_range speed_roll {0.f,0.f};

		Float_range initial_speed {1.f,1.f};
		Float_range final_speed {1.f,1.f};

		Attractor_plane attractor_plane;
		Attractor_point attractor_point;
	};
	using Particle_type_ptr = asset::Ptr<Particle_type>;

	class Particle_emitter {
	public:
		Particle_emitter() = default;
		virtual ~Particle_emitter() = default;

		void position(glm::vec3 position) {
			_position = position;
		}
		void direction(glm::vec3 euler_angles) {
			_direction = glm::quat(euler_angles);
		}

		virtual auto texture()const noexcept -> const Texture* = 0;
		virtual void update(Time dt) = 0;
		virtual void draw(Command& cmd)const = 0;
		virtual void disable() {_active = false;}
		virtual bool dead()const noexcept {return !_active;}

	protected:
		glm::vec3 _position;
		glm::quat _direction;
		bool _active = true;
	};
	using Particle_emitter_ptr = std::shared_ptr<Particle_emitter>;

	extern void set_position(Particle_emitter&, glm::vec3 position);
	extern void set_direction(Particle_emitter&, glm::vec3 euler_angles);

	class Particle_renderer {
		public:
			Particle_renderer(Engine&);

			auto create_emiter(Particle_type_id) -> Particle_emitter_ptr;

			void update(Time dt);
			void draw(Command_queue&)const;

		private:
			std::unordered_map<Particle_type_id, Particle_type_ptr> _types;
			std::vector<Particle_emitter_ptr> _emitters;

			mutable Shader_program _simple_shader;
	};

}
}
