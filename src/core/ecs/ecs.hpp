/** The core part of the entity-component-system *****************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#define ECS_INCLUDED

#include "../utils/log.hpp"
#include "../utils/maybe.hpp"
#include "../utils/string_utils.hpp"
#include "../utils/template_utils.hpp"

#include <unordered_map>
#include <deque>
#include <queue>
#include <memory>

#include "component.hpp"

// forward declarations
namespace sf2 {
	namespace io {
		class CharSource;
		class CharSink;
	}
}

namespace lux {
	namespace asset {
		class AID;
		class Asset_manager;
	}

namespace ecs {
	class Entity_manager;
	class Serializer;

	using Component_filter = std::function<bool(Component_type)>;

	namespace details {
		using Comp_add_function = std::function<void(Entity& e)>;
		using Comp_get_function = std::function<Component_base*(Entity& e)>;

		struct Component_type_info {
			std::string name;
			Component_type type;
			Component_pool_base* pool;
			Comp_add_function add;
			Comp_get_function get;
		};
	}


	class Entity : public std::enable_shared_from_this<Entity>, util::no_copy_move {
		public:
			template<typename T>
			util::maybe<T&> get();

			template<typename T>
			util::maybe<T&> getByType(Component_type type);

			template<typename T>
			bool has();

			template<typename T, typename... Args>
			auto emplace(Args&&... args) -> T&;

			template<typename T>
			void erase();

			template<typename... T>
			void erase_other();

			template<typename T>
			auto get_handle() -> util::lazy<util::maybe<T&>>;

			auto manager() -> Entity_manager& {return _manager;}

		protected:
			Entity(Entity_manager& em) : _manager(em) {}
			friend details::Component_base*& details::get_component(Entity& e, Component_type t);

			Entity_manager& _manager;
			details::Component_base* _components[details::max_comp_type] = {nullptr};
	};

	extern auto entity_name(Entity_ptr) -> std::string;


	// entity transfer object
	using ETO = std::string;

	class Entity_manager : util::no_copy_move {
		public:
			Entity_manager(asset::Asset_manager& asset_mgr);

			auto emplace()noexcept -> Entity_ptr;
			auto emplace(const asset::AID& blueprint)noexcept -> Entity_ptr;
			void erase(Entity_ptr entity);

			template<typename Comp>
			auto list() -> typename Comp::Pool&;

			template<typename T>
			void register_component_type();
			auto comp_info(const std::string& name)const -> const details::Component_type_info&;
			auto find_comp_info(const std::string& name)const -> util::maybe<const details::Component_type_info&>;
			auto list_comp_infos()const {return util::range(_types);}

			void process_queued_actions();
			void shrink_to_fit();

			auto backup(Entity_ptr source) -> std::string;
			void restore(Entity_ptr target, const std::string& data);
			auto restore(const std::string& data) -> Entity_ptr;

			void write(std::ostream&, Component_filter filter={});
			void write(std::ostream&, const std::vector<Entity_ptr>&, Component_filter filter={});
			void read(std::istream&, bool clear=true, Component_filter filter={});

		private:
			friend class Entity;
			friend auto load_entity(Entity_manager& manager, const ETO& eto) -> Entity_ptr;

			asset::Asset_manager& _asset_mgr;

			std::vector<Entity_ptr> _entities;
			std::vector<Entity_ptr> _delete_queue;
			unsigned int _unoptimized_deletions;

			std::unique_ptr<Component_pool_base> _pools[details::max_comp_type];
			std::unordered_map<std::string, details::Component_type_info> _types;
	};

} /* namespace ecs */
}

#include "ecs.hxx"

