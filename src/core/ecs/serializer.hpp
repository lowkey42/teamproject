/** Reads & writes entites to disc *******************************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <string>
#include <unordered_map>

#include "ecs.hpp"
#include "../asset/asset_manager.hpp"

#include <sf2/sf2.hpp>


namespace lux {
namespace asset {
	class AID;
	class Asset_manager;
}

namespace ecs {

	struct EcsSerializer : public sf2::JsonSerializer {
		EcsSerializer(std::ostream& stream, Entity_manager& m,
		              asset::Asset_manager& assets,
		              Component_filter filter={})
			: sf2::JsonSerializer(stream),
			  manager(m), assets(assets), filter(filter) {
		}

		Entity_manager& manager;
		asset::Asset_manager& assets;
		Component_filter filter;
	};
	struct EcsDeserializer : public sf2::JsonDeserializer {
		EcsDeserializer(const std::string& source_name,
		                std::istream& stream, Entity_manager& m,
		                asset::Asset_manager& assets,
		                Component_filter filter={});

		Entity_manager& manager;
		asset::Asset_manager& assets;
		Component_filter filter;
	};

	extern Component_type blueprint_comp_id;
/*
	extern void load(sf2::JsonDeserializer& s, Entity& e);
	extern void save(sf2::JsonSerializer& s, const Entity& e);
	extern void load(sf2::JsonDeserializer& s, Entity_ptr& e); // deprecated?


	class Blueprint;

	extern void init_blueprints(Entity_manager&);

	extern void apply_blueprint(asset::Asset_manager&, Entity& e,
	                            asset::AID blueprint);
								*/
}
}
