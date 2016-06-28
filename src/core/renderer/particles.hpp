/** 3D particle system *******************************************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke                                          *
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
		Float_range initial_opacity {1.f,1.f}; // TODO synonym/spelling?
		Float_range final_opacity {1.f,1.f};

		Float_range initial_size {0.2f,0.2f};
		Float_range final_size {0.2f,0.2f};
		Float_range rotation {0.f,0.f};
		Float_range lifetime {1.f,1.f};

		Float_range emision_rate {100.f, 100.f}; // TODO synonym/spelling?
		int max_particle_count = 1000;

		// TODO: configurable shape
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
	using Particle_type_ptr = std::shared_ptr<const Particle_type>;

	class Particle_emiter;
	using Particle_emiter_ptr = std::shared_ptr<Particle_emiter>;

	extern void set_position(Particle_emiter&, glm::vec3 position);
	extern void set_direction(Particle_emiter&, glm::vec3 eular_angles);

	class Particle_renderer {
		public:
			Particle_renderer(Engine&);

			auto create_emiter(Particle_type_id) -> Particle_emiter_ptr;

			void update(Time dt);
			void draw(Command_queue&)const;

		private:
			std::unordered_map<Particle_type_id, Particle_type_ptr> _types;
			std::vector<Particle_emiter_ptr> _emiters;

			Shader_program _simple_shader;
	};

}
}
