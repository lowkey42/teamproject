/** Basic types, e.g. handles for entities & components **********************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once
#define ECS_COMPONENT_INCLUDED

#include "types.hpp"

#include "../utils/maybe.hpp"
#include "../utils/log.hpp"
#include "../utils/pool.hpp"

#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>


namespace lux {
namespace ecs {

	template<class T>
	class Component_container;


	struct Index_policy {
		void attach(Entity_id, Component_index);
		void detach(Entity_id);
		void shrink_to_fit();
		auto find(Entity_id) -> util::maybe<Component_index>;
		void clear();
	};
	class Sparse_index_policy;  //< for rarely used components
	class Compact_index_policy; //< for frequently used components

	template<class T>
	struct Storage_policy {
		using iterator = void;
		auto begin() -> iterator;
		auto end() -> iterator;

		template<class... Args>
		auto emplace(Args&&... args) -> std::tuple<T&, Component_index>;
		void replace(Component_index, T&&);
		void erase(Component_index);
		void shrink_to_fit();
		auto get(Component_index) -> T&;
		void clear();
	};
	template<std::size_t Chunk_size, class T>
	class Pool_storage_policy;

	// TODO: remove number of moves required for component management (mainly relocation and insertion)


	/**
	 * Optional base class for components.
	 * All components have to be default-constructable, move-assignable and provide a storage_policy,
	 *   index_policy and a name and a constructor taking only User_data&, Entity_manager&, Entity_handle.
	 *
	 * A component is expected to be reasonable lightweight (i.e. cheap to move).
	 * Any component C may provide the following additional ADL functions for serialisation:
	 *  - void load_component(ecs::Deserializer& state, C& v)
	 *  - void save_component(ecs::Serializer& state, const C& v)
	 */
	template<class T, class Index_policy=Sparse_index_policy, class Storage_policy=Pool_storage_policy<32, T>>
	class Base_component {
		public:
			using index_policy   = Index_policy;
			using storage_policy = Storage_policy;
			using Pool           = Component_container<T>;
			// static constexpr auto name = "Component";

			Base_component() : _manager(nullptr), _owner(invalid_entity) {}
			Base_component(User_data&, Entity_manager& manager, Entity_handle owner)
			    : _manager(&manager), _owner(owner) {}

			auto owner_handle()const noexcept -> Entity_handle {
				INVARIANT(_owner!=0, "invalid component");
				return _owner;
			}
			auto manager() noexcept -> Entity_manager& {
				INVARIANT(_manager, "invalid component");
				return *_manager;
			}
			auto owner_facet() -> Entity_facet {
				return {manager(), owner_handle()};
			}

		protected:
			~Base_component()noexcept = default; //< protected destructor to avoid destruction by base-class

		private:
			Entity_manager* _manager;
			Entity_handle _owner;
	};


	/**
	 * All operations except for emplace_now, find_now and process_queued_actions are deferred
	 *   and inherently thread safe.
	 */
	class Component_container_base {
		friend class Entity_manager;
		protected:
			Component_container_base() = default;
			Component_container_base(Component_container_base&&) = delete;
			Component_container_base(const Component_container_base&) = delete;
			virtual ~Component_container_base() = default;

			//< NOT thread-safe
			virtual void* emplace_or_find_now(Entity_handle owner) = 0;

			//< NOT thread-safe
			virtual util::maybe<void*> find_now(Entity_handle owner) = 0;

			//< NOT thread-safe
			virtual void process_queued_actions() = 0;

			/// thread safe
			virtual void clear() = 0;

			/// thread safe
			virtual void erase(Entity_handle owner) = 0;

			/// thread safe
			// auto find(Entity_handle owner) -> util::maybe<T&>

			/// thread safe
			// void emplace(Entity_handle owner, Args&&... args);

		public:
			// begin()
			// end()
			// size()
			// empty()
	};

} /* namespace ecs */
}

#include "component.hxx"
