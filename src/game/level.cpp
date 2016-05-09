#include "level.hpp"

#include "sys/physics/transform_comp.hpp"
#include "sys/graphic/terrain_comp.hpp"

#include <core/ecs/serializer.hpp>
#include <core/asset/asset_manager.hpp>
#include <core/utils/sf2_glm.hpp>

namespace lux {

	namespace {
		struct Level_list {
			std::vector<std::string> levels;
		};

		sf2_structDef(Level_list,
			levels
		)
	}

	sf2_structDef(Level_info,
		id,
		name,
		author,
		description
	)

	sf2_structDef(Level_data,
		id,
		name,
		author,
		description,
		environment_id,
		environment_light_color,
		environment_light_direction,
		ambient_brightness,
		background_tint
	)

	namespace {
		auto level_aid(const std::string& id) {
			return asset::AID{"level"_strid, "level_"+id+".map"};
		}

		void add_to_list(Engine& engine, const std::string& id) {
			auto level_list = engine.assets().load_maybe<Level_list>("cfg:my_levels"_aid).process(Level_list{}, [](auto p){return *p;});

			if(std::find(level_list.levels.begin(), level_list.levels.end(),
			             id) == level_list.levels.end()) {

				level_list.levels.push_back(id);
				engine.assets().save<Level_list>("cfg:my_levels"_aid, level_list);
			}
		}
	}

	auto list_local_levels(Engine& engine) -> std::vector<Level_info_ptr> {
		auto level_list = engine.assets().load<Level_list>("cfg:levels"_aid);
		auto my_level_list = engine.assets().load_maybe<Level_list>("cfg:my_levels"_aid).process(Level_list{}, [](auto p){return *p;});

		auto ret = std::vector<Level_info_ptr>();
		ret.reserve(level_list->levels.size() + my_level_list.levels.size());
		for(auto& lid : level_list->levels) {
			ret.emplace_back(engine.assets().load<Level_info>(level_aid(lid)));
		}
		for(auto& lid : my_level_list.levels) {
			ret.emplace_back(engine.assets().load<Level_info>(level_aid(lid)));
		}

		return ret;
	}
	auto load_level(Engine& engine, ecs::Entity_manager& ecs, const std::string& id) -> Level_data {
		auto data = engine.assets().load_raw(level_aid(id));

		INVARIANT(!data.is_nothing(), "Level doesn't exists: "<<id);

		auto& stream = data.get_or_throw();

		Level_data level_data;
		sf2::deserialize_json(stream, [&](auto& msg, uint32_t row, uint32_t column) {
			ERROR("Error parsing LevelData from "<<id<<" at "<<row<<":"<<column<<": "<<msg);
		}, level_data);

		ecs.read(stream, true);

		return level_data;
	}

	void save_level(Engine& engine, ecs::Entity_manager& ecs, const Level_data& level) {
		auto stream = engine.assets().save_raw(level_aid(level.id));

		sf2::serialize_json(stream, level);

		stream<<std::endl; // line-break and flush

		ecs.write(stream, +[](ecs::Component_type comp) {
			return comp==sys::physics::Transform_comp::type()
			        || comp==ecs::blueprint_comp_id
			        || comp==sys::graphic::Terrain_data_comp::type();
		});

		stream.close();

		add_to_list(engine, level.id);
	}


	auto list_remote_levels(Engine&, int32_t count, int32_t offset) -> std::vector<Level_info> {
		// TODO
		return {};
	}
	auto download_level(Engine&, const std::string& id) -> std::future<void> {
		// TODO
		std::promise<void> dummy;
		auto future = dummy.get_future();
		dummy.set_value();
		return future;
	}
	void upload_level(Engine&, const std::string& id) {
		// TODO
	}

}
