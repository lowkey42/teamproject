/** a resizable, semi-contiguous pool of memory ******************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include "log.hpp"
#include "string_utils.hpp"
#include "template_utils.hpp"


namespace lux {
namespace util {

	template<class POOL>
	class pool_iterator;


	struct pool_value_traits {
		static constexpr bool supports_empty_values = false;
		// static constexpr int_fast32_t max_free = 8;
		// using Marker_type = Foo;
		// static constexpr Marker_type free_mark = -1;
		// static constexpr const Marker_type* marker_addr(const T* inst)
	};

	template<class T, std::size_t ElementsPerChunk, class IndexType=int_fast64_t,
	         class ValueTraits=pool_value_traits, bool use_empty_values=ValueTraits::supports_empty_values>
	class pool {
		static_assert(alignof(T)<=alignof(std::max_align_t), "Alignment not supported");
		public:
			static constexpr auto element_size = static_cast<IndexType>(sizeof(T));
			static constexpr auto chunk_len    = static_cast<IndexType>(ElementsPerChunk);
			static constexpr auto chunk_size   = chunk_len * element_size;

			using value_type = T;
			using iterator = pool_iterator<pool<T, ElementsPerChunk, IndexType, ValueTraits, use_empty_values>>;
			using index_t = IndexType;

			friend iterator;


			pool()noexcept = default;
			pool(pool&&)noexcept = default;
			pool& operator=(pool&&)noexcept = default;
			~pool() = default;

			iterator begin()noexcept;
			iterator end()noexcept;

			/**
			 * Deletes all elements. Invalidates all iterators and references.
			 * O(N)
			 */
			void clear()noexcept {
				for(auto& chunk : _chunks) {
					for(IndexType i=0; i<chunk_len; i++) {
						chunk[i].~T();
					}
				}
				_chunks.clear();
				_used_elements = 0;
			}
			/**
			 * Deletes the last element. Invalidates all iterators and references to the last element.
			 * O(1)
			 */
			void pop_back() {
				INVARIANT(_used_elements>0, "pop_back on empty pool");
#ifdef _NDEBUG
				std::memset(get(_usedElements-1), 0xdead, element_size);
#endif
				_used_elements--;
				get(_used_elements).~T();
			}
			/**
			 * Deletes a specific element.
			 * Invalidates all iterators and references to the specified and the last element.
			 * O(1)
			 */
			auto erase(IndexType i) {
				INVARIANT(i<_used_elements, "erase is out of range");

				if(i<(_used_elements-1)) {
					get(i) = std::move(back());
				}

				pop_back();
			}

			/**
			 * Frees all unused memory. May invalidate all iterators and references.
			 * O(1)
			 */
			void shrink_to_fit() {
				auto min_chunks = std::ceil(static_cast<float>(_used_elements)/chunk_len);
				_chunks.resize(static_cast<std::size_t>(min_chunks));
			}

			/**
			 * Creates a new instance of T inside the pool.
			 * O(1)
			 */
			template<class... Args>
			auto emplace_back(Args&&... args) -> std::tuple<T&, IndexType> {
				const auto i = _used_elements++;

				auto addr = [&] {
					auto chunk = i/chunk_len;

					if(chunk < _chunks.size()) {
						return _chunks[chunk][(i % chunk_len) * element_size];

					} else {
						auto new_chunk = std::make_unique<unsigned char[]>(chunk_size);
						auto addr = new_chunk.get();
						_chunks.push_back(std::move(new_chunk));
						return addr;
					}
				}();

				auto instnace = new(addr) T(std::forward<Args>(args)...);
				return {*instnace, i};
			}

			void replace(IndexType i, T&& new_element) {
				get(i) = std::move(new_element);
			}

			/**
			 * @return The number of elements in the pool
			 */
			IndexType size()const noexcept {
				return _used_elements;
			}
			bool empty()const noexcept {
				return _used_elements!=0;
			}

			/**
			 * @return The specified element
			 */
			T& get(IndexType i) {
				return static_cast<T&>(*get_raw(i));
			}
			const T& get(IndexType i)const {
				return static_cast<const T&>(*get_raw(i));
			}

			/**
			 * @return The last element
			 */
			const T& back() {
				return const_cast<T&>(static_cast<const pool*>(this)->back());
			}
			const T& back()const {
				INVARIANT(_used_elements>0, "back on empty pool");
				auto i = _used_elements-1;
				return reinterpret_cast<const T&>(_chunks.back()[(i % chunk_len) * element_size]);
			}

		protected:
			using chunk_type = std::unique_ptr<unsigned char[]>;
			std::vector<chunk_type> _chunks;
			IndexType _used_elements = 0;

			// get_raw is required to avoid UB if their is no valid object at the index
			char* get_raw(IndexType i) {
				return const_cast<char*>(static_cast<const pool*>(this)->get_raw(i));
			}
			const char* get_raw(IndexType i)const {
				INVARIANT(i<_used_elements, "Pool-Index out of bounds "+to_string(i)+">="+to_string(_used_elements));

				return _chunks[i / chunk_len].get() + (i % chunk_len) * element_size;
			}

			T* _chunk(IndexType chunk_idx)noexcept {
				if(chunk_idx<static_cast<IndexType>(_chunks.size()))
					return reinterpret_cast<T*>(_chunks[chunk_idx].get());
				else
					return nullptr;
			}
			T* _chunk_end(IndexType chunk_idx)noexcept {
				if(chunk_idx < _used_elements/chunk_len) {
					return reinterpret_cast<T*>(_chunks[chunk_idx].get() + chunk_len);
				} else {
					return reinterpret_cast<T*>(_chunks[chunk_idx].get() + (_used_elements % chunk_len));
				}
			}
			static bool _valid(const T*)noexcept {
				return true;
			}
	};
	
	template<class T, std::size_t ElementsPerChunk, class IndexType, class ValueTraits>
	class pool<T, ElementsPerChunk, IndexType, ValueTraits, true>
	        : public pool<T, ElementsPerChunk, IndexType, ValueTraits, false> {

		using base_t = pool<T, ElementsPerChunk, IndexType, ValueTraits, false>;
		public:
			using iterator = pool_iterator<pool<T, ElementsPerChunk, IndexType, ValueTraits, true>>;

			friend iterator;

			iterator begin()noexcept;
			iterator end()noexcept;

			void clear()noexcept {
				for(auto& chunk : this->_chunks) {
					for(IndexType i=0; i<this->chunk_len; i++) {
						if(_valid(chunk+i)) {
							chunk[i].~T();
						}
					}
				}
				this->_chunks.clear();
				this->_used_elements = 0;

				_freelist.clear();
			}
			
			auto erase(IndexType i) {
				if(i>=(this->size()-1)) {
					this->pop_back();

				} else {
					T& instance = get(i);
					auto instance_addr = &instance;
					INVARIANT(_valid(instance_addr), "double free");
					instance.~T();
					set_free(instance_addr);

					_freelist.emplace_back(i);
				}
			}
			
			template<class... Args>
			auto emplace_back(Args&&... args) -> std::tuple<T&, IndexType> {
				if(!_freelist.empty()) {
					auto i = _freelist.back();
					_freelist.pop_back();

					auto instance_addr = get_raw(i);
					INVARIANT(!_valid(instance_addr), "Freed object is not marked as free");
					auto instance = (new(instance_addr) T(std::forward<Args>(args)...));
					return {*instance, i};
				}

				return base_t::emplace_back(std::forward<Args>(args)...);
			}
			
			void shrink_to_fit() {
				if(_freelist.size() > ValueTraits::max_free) {
					std::sort(_freelist.begin(), _freelist.end(), std::greater<IndexType>{});
					for(auto i : _freelist) {
						base_t::erase(i);
					}
					_freelist.clear();
				}

				base_t::shrink_to_fit();
			}
			
			
		protected:
			std::vector<IndexType> _freelist;

			static auto& get_marker(const T* obj)noexcept {
				return *ValueTraits::marker_addr(obj);
			}
			static void set_free(const T* obj)noexcept {
				const_cast<typename ValueTraits::Marker_type>(get_marker(obj)) = ValueTraits::free_mark;
			}
			static bool _valid(const T* obj)noexcept {
				if(obj==nullptr)
					return true;
				
				return get_marker(obj) != ValueTraits::free_mark;
			}
	};
	
	
	template<class Pool>
	class pool_iterator : public std::iterator<std::bidirectional_iterator_tag, typename Pool::value_type> {
		public:
			using value_type = typename Pool::value_type;
			
			pool_iterator()
			    : _pool(nullptr),
			      _chunk_index(0),
			      _element_iter(nullptr),
			      _element_iter_begin(nullptr),
			      _element_iter_end(nullptr) {}

			pool_iterator(Pool& pool, typename Pool::index_t index=0)
			    : _pool(&pool),
			      _chunk_index(index/Pool::chunk_len),
			      _element_iter(_pool->_chunk(index) + (index%Pool::chunk_len)),
			      _element_iter_begin(pool._chunk(_chunk_index)),
			      _element_iter_end(pool._chunk_end(_chunk_index)) {}

			value_type& operator*()noexcept {return *_element_iter;}
			value_type* operator->()noexcept {return _element_iter;}

			pool_iterator& operator++() {
				do {
					++_element_iter;
					if(_element_iter==_element_iter_end) {
						++_chunk_index;
						_element_iter_begin = _element_iter = _pool->_chunk(_chunk_index);
						_element_iter_end = _pool->_chunk_end(_chunk_index);
					}
				} while(!Pool::_valid(_element_iter));

				return *this;
			}

			pool_iterator operator++(int) {
				pool_iterator t = *this;
				++*this;
				return t;
			}
			
			pool_iterator& operator--() {
				do {
					if(_element_iter==_element_iter_begin) {
						INVARIANT(_chunk_index>0, "iterator underflow");
						--_chunk_index;
						_element_iter_begin = _element_iter = _pool->_chunk(_chunk_index);
						_element_iter_end = _pool->_chunk_end(_chunk_index);
					} else {
						--_element_iter;
					}
				} while(!Pool::_valid(_element_iter));

				return *this;
			}

			pool_iterator operator--(int) {
				pool_iterator t = *this;
				--*this;
				return t;
			}


			bool operator==(const pool_iterator& o) const {
				return _element_iter==o._element_iter;
			}
			bool operator!=(const pool_iterator& o) const {
				return !(*this==o);
			}

		private:
			Pool* _pool;
			typename Pool::index_t _chunk_index;
			value_type* _element_iter;
			value_type* _element_iter_begin;
			value_type* _element_iter_end;
	};

	template<class T, std::size_t ElementsPerChunk, class Index_type, class ValueTraits, bool use_empty_values>
	auto pool<T, ElementsPerChunk, Index_type, ValueTraits, use_empty_values>::begin()noexcept -> iterator {
		return iterator{*this};
	}

	template<class T, std::size_t ElementsPerChunk, class Index_type, class ValueTraits, bool use_empty_values>
	auto pool<T, ElementsPerChunk, Index_type, ValueTraits, use_empty_values>::end()noexcept -> iterator {
		return iterator{};
	}


	template<class T, std::size_t ElementsPerChunk, class Index_type, class ValueTraits>
	auto pool<T, ElementsPerChunk, Index_type, ValueTraits, true>::begin()noexcept -> iterator {
		return iterator{*this};
	}

	template<class T, std::size_t ElementsPerChunk, class Index_type, class ValueTraits>
	auto pool<T, ElementsPerChunk, Index_type, ValueTraits, true>::end()noexcept -> iterator {
		return iterator{};
	}

}
}

