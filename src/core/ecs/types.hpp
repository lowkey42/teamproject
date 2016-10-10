/** Basic types, e.g. handles for entities & components **********************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "../utils/maybe.hpp"
#include "../utils/pool.hpp"

#include <moodycamel/concurrentqueue.hpp>

#include <cstdint>
#include <vector>
#include <string>


namespace lux {
	class Engine;

	namespace ecs {
		using User_data = ::lux::Engine;
	}
}

namespace lux {
namespace ecs {

	class Entity_manager;
	class Entity_facet;

	using Component_index = int32_t;
	using Component_type = int_fast16_t;

	namespace detail {
		extern Component_index id_generator();
	}

	template <typename t>
	auto component_type_id() {
		static const auto type_id = detail::id_generator();
		return type_id;
	}


	using Entity_id = int32_t;
	struct Entity_handle {
		Entity_id id;
		uint8_t revision; // 0b10000000 marks handle as free

		constexpr Entity_handle() : id(-1), revision(0) {}
		constexpr Entity_handle(Entity_id id, uint8_t revision) : id(id), revision(revision) {}
	};

	constexpr auto invalid_entity = Entity_handle{};


	extern auto get_entity_id(Entity_handle h, Entity_manager&) -> Entity_id;
	extern auto entity_name(Entity_handle h) -> std::string;

	class Entity_handle_generator {
		using Freelist = moodycamel::ConcurrentQueue<Entity_handle>;
		public:
			Entity_handle_generator(Entity_id max=128) {
				_slots.resize(max);
			}

			// thread-safe
			auto get_new() -> Entity_handle {
				Entity_handle h;
				if(_free.try_dequeue(h)) {
					auto& rev = _slots.at(h.id);
					rev.store(rev & (~0b10000000)); // mark as used
					return h;
				}

				auto slot = _next_free_slot++;

				return join_entity_handle(slot, 0);
			}

			// thread-safe
			auto valid(Entity_handle h)const noexcept -> bool {
				return _slots.size()<=h.id || _slots[h.id] == h.revision;
			}

			// NOT thread-safe
			auto free(Entity_handle h) -> Entity_handle {
				if(h.id>=_slots.size()) {
					_slots.resize(h.id*2);
				}

				auto rev = _slots.at(h.id).load();
				_slots.at(h.id).store(rev | 0b10000000); // mark as free; no CAS required only written here and in get_new() if already in _free

				_free.enqueue(h.id, rev);
				return {h.id, rev};
			}

			// NOT thread-safe
			template<typename F>
			void foreach_valid_handle(F&& func) {
				auto end = _next_free_slot.load();

				for(Entity_id id=0; id<end) {
					if(id>=_slots.size())
						func(Entity_handle{id,0});
					else {
						auto rev = _slots[id].load();
						if((rev & ~0b10000000)!=0) { // is marked used
							func(Entity_handle{id,rev});
						}
					}
				}
			}

			// NOT thread-safe
			void clear() {
				_slots.clear();
				_slots.resize(_slots.capacity());
				_next_free_slot = 0;
				_free = Freelist{}; // clear by moving a new queue into the old
			}

		private:
			std::vector<std::atomic<uint8_t>> _slots;
			std::atomic<Entity_id> _next_free_slot = 0;
			Freelist _free;
	};


	/**
	 * Thread-safe facet to a single entity.
	 *
	 * All mutations (emplace, erase, erase_other) are deferred which causes the following limitations:
	 *   - get/has may not reflect the most recent state of the entity
	 *   - the behavious is undefined if a component is emplaced and erased during the same frame
	 *   - the order of operations is not guaranteed
	 */
	class Entity_facet {
		public:
			Entity_facet() : _manager(nullptr), _owner(invalid_entity) {}
			Entity_facet(Entity_manager& manager, Entity_handle owner)
			    : _manager(&manager), _owner(owner) {}

			template<typename T>
			util::maybe<T&> get();

			template<typename T>
			bool has();

			template<typename T, typename... Args>
			auto emplace(Args&&... args) -> T&;

			template<typename T>
			void erase();

			template<typename... T>
			void erase_other();

			auto manager() -> Entity_manager& {return *_manager;}
			auto handle()const {return _owner;}

			auto valid()const noexcept -> bool;

			// see ecs.hxx for implementation of template methods

		private:
			Entity_manager* _manager;
			Entity_handle _owner;
	};



} /* namespace ecs */
}

