/** Structures and access functions for level descriptions and data **********
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once


#include <core/ecs/ecs.hpp>
#include <core/engine.hpp>

#include <future>


namespace lux {

	// TODO: thumbnails

	struct Level_info {
		std::string id;
		std::string name;
		std::string author;
		std::string description;
	};
	using Level_info_ptr = std::shared_ptr<const Level_info>;

	struct Level_data : Level_info {
		std::string environment_id;
		Rgb environment_light_color;
		glm::vec3 environment_light_direction;
		float ambient_brightness;
		std::string entity_data;
	};
	using Level_data_ptr = std::shared_ptr<const Level_data>;

	extern auto list_local_levels(Engine&) -> std::vector<Level_info_ptr>;
	extern auto load_level(Engine&, ecs::Entity_manager& ecs, const std::string& id) -> Level_data;

	extern void save_level(Engine&, ecs::Entity_manager& ecs, const Level_data&);


	extern auto list_remote_levels(Engine&, int32_t count, int32_t offset) -> std::vector<Level_info>;
	extern auto download_level(Engine&, const std::string& id) -> std::future<void>;
	extern void upload_level(Engine&, const std::string& id);

}
