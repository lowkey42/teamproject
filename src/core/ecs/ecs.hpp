/** The core part of the entity-component-system *****************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#define ECS_INCLUDED

#include "component.hpp"
#include "types.hpp"

#include "../utils/log.hpp"
#include "../utils/maybe.hpp"
#include "../utils/string_utils.hpp"
#include "../utils/template_utils.hpp"

#include <moodycamel/concurrentqueue.hpp>

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>


namespace lux {
namespace ecs {
	class Entity_manager;
	class Serializer;

	using Component_filter = std::function<bool(Component_type)>;


	// entity transfer object
	using ETO = std::string;

	/**
	 * The main functionality is thread-safe but the other methods require a lock to prevent
	 *   concurrent read access during their execution
	 */
	class Entity_manager : util::no_copy_move {
		public:
			Entity_manager(User_data& userdata);

		// user interface; thread-safe
			auto emplace()noexcept -> Entity_facet;
			auto emplace(const std::string& blueprint) -> Entity_facet;
			auto get(Entity_handle entity) -> util::maybe<Entity_facet>;
			auto validate(Entity_handle entity) -> bool {
				return _handles.valid(entity);
			}

			// deferred to next call to process_queued_actions
			void erase(Entity_handle entity);

			template<typename C>
			auto list() -> Component_container<C>&;

			auto& userdata()noexcept {return _userdata;}

		// serialization interface; not thread-safe (yet?)
			auto write_one(Entity_handle source) -> ETO;
			auto read_one(ETO data, Entity_handle target=invalid_entity) -> Entity_facet;

			void write(std::ostream&, Component_filter filter={});
			void write(std::ostream&, const std::vector<Entity_handle>&, Component_filter filter={});
			void read(std::istream&, bool clear=true, Component_filter filter={});


		// manager/engine interface; not thread-safe
			void process_queued_actions();
			void clear();
			template<typename T>
			void register_component_type();

		private:
			friend class Entity_facet;

			struct Component_type_info {
				std::string name;
				Component_type type;
				Component_container_base* pool;
			};

			using Erase_queue = moodycamel::ConcurrentQueue<Entity_handle>;

			User_data& _userdata;

			Entity_handle_generator _handles;
			Erase_queue _queue_erase;

			std::vector<std::unique_ptr<Component_container_base>> _components;
			std::unordered_map<std::string, Component_type_info>   _components_by_name;
	};

} /* namespace ecs */
}

#include "ecs.hxx"

