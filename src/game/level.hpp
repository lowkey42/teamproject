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

	struct Level_info {
		std::string id;
		std::string name;
		std::string author;
		std::string description;
		std::string pack;

		std::string environment_id;
		float environment_brightness = 1.f;
		Rgb environment_light_color;
		glm::vec3 environment_light_direction;
		float ambient_brightness;
		Rgba background_tint {1,1,1,0};
		std::string music_id;
	};
	using Level_info_ptr = std::shared_ptr<const Level_info>;

	struct Level_pack_entry {
		std::string aid;
		glm::vec2 position;
	};
	struct Level_pack {
		std::string name;
		std::string description;
		std::vector<Level_pack_entry> level_ids;
		std::string icon_texture_id;
		std::string map_texture_id;

		auto find_level(std::string id)const -> util::maybe<int>;
	};
	using Level_pack_ptr = std::shared_ptr<const Level_pack>;


	extern auto list_local_levels(Engine&) -> std::vector<Level_info_ptr>;
	extern auto get_level(Engine&, const std::string& id) -> Level_info_ptr;
	extern auto load_level(Engine&, ecs::Entity_manager& ecs, const std::string& id) -> Level_info;
	extern void save_level(Engine&, ecs::Entity_manager& ecs, const Level_info&);

	extern auto list_level_packs(Engine&) -> std::vector<Level_pack_ptr>;
	extern auto get_level_pack(Engine&, const std::string& id) -> Level_pack_ptr;

	extern void unlock_level(Engine&, const std::string& id);
	extern void unlock_next_levels(Engine&, const std::string& id);
	extern auto is_level_locked(Engine&, const std::string& id) -> bool;

	// poss. move to own hpp/cpp
	extern auto list_remote_levels(Engine&, int32_t count, int32_t offset) -> std::vector<Level_info>;
	extern auto download_level(Engine&, const std::string& id) -> std::future<void>;
	extern void upload_level(Engine&, const std::string& id);

}
