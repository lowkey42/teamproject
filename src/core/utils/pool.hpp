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

	// TODO: make thread-safe
	template<class T, std::size_t ElementsPerChunk, class Index_type=int_fast64_t>
	class pool {
		static_assert(alignof(T)<=alignof(std::max_align_t), "Alignment not supported");
		friend class pool_iterator<pool<T, ElementsPerChunk>>;
		public:
			static constexpr auto element_size = static_cast<Index_type>(sizeof(T));
			static constexpr auto chunk_len    = static_cast<Index_type>(ElementsPerChunk);
			static constexpr auto chunk_size   = chunk_len * element_size;

			using value_type = T;
			using iterator = pool_iterator<pool<T, ElementsPerChunk>>;
			using index_t = Index_type;


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
					for(Index_type i=0; i<chunk_len; i++) {
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
			auto erase(Index_type i) {
				INVARIANT(i<_used_elements, "erase is out of range");

				if(i<(_used_elements-1)) {
					get(i) = std::move(back());
				}

				pop_back();
			}

			/**
			 * Frees all unused memory.
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
			auto emplace_back(Args&&... args) -> std::tuple<T&, Index_type> {
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

			void replace(Index_type i, T&& new_element) {
				get(i) = std::move(new_element);
			}

			/**
			 * @return The number of elements in the pool
			 */
			Index_type size()const noexcept {
				return _used_elements;
			}
			bool empty()const noexcept {
				return _used_elements!=0;
			}

			/**
			 * @return The specified element
			 */
			T& get(Index_type i) {
				return const_cast<T&>(static_cast<const pool*>(this)->get(i));
			}
			const T& get(Index_type i)const {
				INVARIANT(i<_used_elements, "Pool-Index out of bounds "+to_string(i)+">="+to_string(_used_elements));

				return static_cast<const T&>(_chunks[i / chunk_len][(i % chunk_len) * element_size]);
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
				return _chunks.back()[(i % chunk_len) * element_size];
			}

		private:
			using chunk_type = std::unique_ptr<unsigned char[]>;
			std::vector<chunk_type> _chunks;
			Index_type _used_elements = 0;

			T* _chunk(Index_type chunk_idx)noexcept {
				if(chunk_idx<_chunks.size())
					return _chunks[chunk_idx];
				else
					return nullptr;
			}
			T* _chunk_end(Index_type chunk_idx)noexcept {
				if(chunk_idx < _used_elements/chunk_len) {
					return _chunks(chunk_idx) + chunk_len;
				} else {
					return _chunks(chunk_idx) + (_used_elements % chunk_len);
				}
			}
	};

	template<class Pool>
	class pool_iterator {
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = typename Pool::value_type;
			using difference_type = std::ptrdiff_t;

			pool_iterator()
			    : _pool(nullptr),
			      _chunk_index(0),
			      _element_iter(nullptr),
			      _element_iter_end(nullptr) {}

			pool_iterator(Pool& pool, typename Pool::index_t index=0)
			    : _pool(&pool),
			      _chunk_index(index/Pool::chunk_len),
			      _element_iter(_pool->_chunk(index) + (index%Pool::chunk_len)),
			      _element_iter_end(pool._chunk_end(_chunk_index)) {}

			value_type operator*()noexcept {return _element_iter;}
			value_type operator->()noexcept {return _element_iter;}

			pool_iterator& operator++() {
				++_element_iter;
				if(_element_iter==_element_iter_end) {
					++_chunk_index;
					_element_iter = _pool->_chunk(_chunk_index);
					_element_iter_end = _pool->_chunk_end(_chunk_index);
				}

				return *this;
			}

			pool_iterator operator++(int) {
				pool_iterator t = *this;
				++*this;
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
			value_type* _element_iter_end;
	};

	template<class T, std::size_t ElementsPerChunk, class Index_type>
	auto pool<T, ElementsPerChunk, Index_type>::begin()noexcept -> iterator {
		return iterator{*this};
	}

	template<class T, std::size_t ElementsPerChunk, class Index_type>
	auto pool<T, ElementsPerChunk, Index_type>::end()noexcept -> iterator {
		return iterator{};
	}

}
}

