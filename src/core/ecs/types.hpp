/** Basic types, e.g. handles for entities & components **********************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "../utils/atomic_utils.hpp"
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

	struct Deserializer;
	struct Serializer;

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
	class Entity_handle {
		public:
			using packed_t = uint32_t;
			static constexpr uint8_t free_rev = 0b10000; // marks revisions as free,
			                                             // is cut off when assigned to Entity_handle::revision

			constexpr Entity_handle() : _id(0), _revision(0) {}
			constexpr Entity_handle(Entity_id id, uint8_t revision) : _id(id), _revision(revision) {}

			operator bool()const noexcept {
				return _id!=0;
			}

			constexpr Entity_id id()const noexcept {
				return _id;
			}
			constexpr void id(Entity_id id)noexcept {
				_id = id;
			}

			constexpr uint8_t revision()const noexcept {
				return _revision;
			}
			constexpr void revision(uint8_t revision)noexcept {
				_revision = revision;
			}

			constexpr bool operator==(const Entity_handle& rhs)const noexcept {
				return _id==rhs._id && _revision==rhs._revision;
			}
			constexpr bool operator!=(const Entity_handle& rhs)const noexcept {
				return _id!=rhs._id || _revision!=rhs._revision;
			}
			bool operator<(const Entity_handle& rhs)const noexcept {
				return std::tie(_id,_revision) < std::tie(rhs._id,rhs._revision);
			}

			packed_t pack()const noexcept {
				return static_cast<packed_t>(_id)<<4 | static_cast<packed_t>(_revision);
			}
			static Entity_handle unpack(packed_t d)noexcept {
				return Entity_handle{static_cast<int32_t>(d>>4), static_cast<uint8_t>(d & 0b1111)};
			}

		private:
			int32_t _id:28;
			uint8_t _revision:4;
	};

	static_assert(sizeof(Entity_handle::packed_t)<=sizeof(void*),
	              "what the hell is wrong with your plattform?!");

	inline Entity_handle to_entity_handle(void* u) {
		return Entity_handle::unpack(static_cast<Entity_handle::packed_t>(
		                                      reinterpret_cast<std::intptr_t>(u)) );
	}
	inline void* to_void_ptr(Entity_handle h) {
		return reinterpret_cast<void*>(static_cast<std::intptr_t>(h.pack()));
	}

	constexpr auto invalid_entity = Entity_handle{};
	constexpr auto invalid_entity_id = invalid_entity.id();


	extern auto get_entity_id(Entity_handle h, Entity_manager&) -> Entity_id;
	extern auto entity_name(Entity_handle h) -> std::string;

	class Entity_handle_generator {
		using Freelist = moodycamel::ConcurrentQueue<Entity_handle>;
		public:
			Entity_handle_generator(Entity_id max=128) {
				_slots.resize(static_cast<std::size_t>(max));
			}

			// thread-safe
			auto get_new() -> Entity_handle {
				Entity_handle h;
				if(_free.try_dequeue(h)) {
					auto& rev = util::at(_slots, static_cast<std::size_t>(h.id()));
					rev.store(rev & ~Entity_handle::free_rev); // mark as used
					return h;
				}

				auto slot = _next_free_slot++;

				return {slot+1, 0};
			}

			// thread-safe
			auto valid(Entity_handle h)const noexcept -> bool {
				return h && (static_cast<Entity_id>(_slots.size()) <= h.id()-1
				             || util::at(_slots, static_cast<std::size_t>(h.id()-1)) == h.revision());
			}

			// NOT thread-safe
			auto free(Entity_handle h) -> Entity_handle {
				if(h.id()-1 >= static_cast<Entity_id>(_slots.size())) {
					_slots.resize(static_cast<std::size_t>(h.id()-1) *2);
				}

				auto rev = util::at(_slots, static_cast<std::size_t>(h.id()-1)).load();

				// mark as free; no CAS required only written here and in get_new() if already in _free
				util::at(_slots, static_cast<std::size_t>(h.id()-1)).store(rev | Entity_handle::free_rev);

				auto handle = Entity_handle{h.id()-1, rev};

				_free.enqueue(handle);
				return {handle};
			}

			// NOT thread-safe
			template<typename F>
			void foreach_valid_handle(F&& func) {
				auto end = _next_free_slot.load();

				for(Entity_id id=0; id<end; id++) {
					if(id>=static_cast<Entity_id>(_slots.size())) {
						func(Entity_handle{id+1,0});
					} else {
						auto rev = util::at(_slots, static_cast<std::size_t>(id)).load();
						if((rev & ~Entity_handle::free_rev)!=0) { // is marked used
							func(Entity_handle{id+1,rev});
						}
					}
				}
			}
			
			auto next(Entity_id curr=-1)const -> Entity_handle {
				auto end = _next_free_slot.load();

				for(Entity_id id=curr+1; id<end; id++) {
					if(id>=static_cast<Entity_id>(_slots.size())) {
						return Entity_handle{id+1,0};
					} else {
						auto rev = util::at(_slots, static_cast<std::size_t>(id)).load();
						if((rev & ~Entity_handle::free_rev)!=0) { // is marked used
							return Entity_handle{id+1,rev};
						}
					}
				}
				
				return invalid_entity;
			}

			// NOT thread-safe
			void clear() {
				_slots.clear();
				_slots.resize(_slots.capacity());
				_next_free_slot = 0;
				_free = Freelist{}; // clear by moving a new queue into the old
			}

		private:
			util::vector_atomic<uint8_t> _slots;
			std::atomic<Entity_id> _next_free_slot{0};
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

			auto manager()noexcept -> Entity_manager& {return *_manager;}
			auto handle()const noexcept {return _owner;}

			auto valid()const noexcept -> bool;

			operator bool()const noexcept {return valid();}
			operator Entity_handle()const noexcept {return handle();}

			void reset() {
				_owner = invalid_entity;
			}
			bool operator==(const Entity_facet& rhs)const noexcept {
				return handle()==rhs.handle();
			}

			// see ecs.hxx for implementation of template methods

		private:
			Entity_manager* _manager;
			Entity_handle _owner;
	};



} /* namespace ecs */
}

