#include "ecs.hpp"

#include <algorithm>
#include <stdexcept>

#include "../utils/string_utils.hpp"
#include "../utils/log.hpp"
#include <iostream>

#include "component.hpp"
#include "serializer.hpp"

#include <sf2/sf2.hpp>

namespace lux {
namespace ecs {

	auto entity_name(Entity_ptr e) -> std::string {
		return e ? util::to_string((void*) e.get()) : "0x0";
	}

	struct Entity_constructor : Entity {
		Entity_constructor(Entity_manager& e) : Entity(e){}
	};


	Entity_manager::Entity_manager(asset::Asset_manager& asset_mgr)
		: _asset_mgr(asset_mgr), _unoptimized_deletions(0) {

		init_blueprints(*this);
	}

	Entity_ptr Entity_manager::emplace()noexcept {
		auto e = std::make_shared<Entity_constructor>(*this);
		_entities.push_back(e);

		return e;
	}
	Entity_ptr Entity_manager::emplace(const asset::AID& blueprint)noexcept {
		auto e = emplace();

		apply_blueprint(_asset_mgr, *e, blueprint);

		return e;
	}

	void Entity_manager::erase(Entity_ptr ref) {
		if(std::find(_delete_queue.begin(), _delete_queue.end(), ref)!=_delete_queue.end()) {
			ERROR("Double-Deletion of entity "<<ref.get());
			return;
		}

		_delete_queue.push_back(ref);
	}

	void Entity_manager::process_queued_actions() {
		constexpr unsigned int resize_after_n_deletions = 50;

		auto new_end = std::remove_if(std::begin(_entities), std::end(_entities),
			[&](Entity_ptr& v){
				bool remove = std::find(_delete_queue.begin(), _delete_queue.end(), v)!=_delete_queue.end();

				if(remove) {
					for(auto& cp : _pools) {
						if(cp)
							cp->free(*v);
					}
				}

				return remove;
		} );

		for(auto& cp : _pools) {
			if(cp)
				cp->process_queued_actions();
		}

		_entities.erase(new_end, _entities.end() );

		_delete_queue.clear();

		if(_unoptimized_deletions>=resize_after_n_deletions) {
			shrink_to_fit();

			_unoptimized_deletions = 0;
		}
	}
	void Entity_manager::shrink_to_fit() {
		for(auto& cp : _pools)
			if(cp)
				cp->shrink_to_fit();
	}

	auto Entity_manager::find_comp_info(const std::string& name)const -> util::maybe<const details::Component_type_info&> {
		auto ti = _types.find(name);
		if(ti!=_types.end())
			return util::justPtr(&ti->second);
		else
			return util::nothing();
	}
	auto Entity_manager::comp_info(const std::string& name)const -> const details::Component_type_info& {
		auto info = find_comp_info(name);
		if(info.is_nothing())
			FAIL("Unknown component: "<<name);

		return info.get_or_throw();
	}

	auto Entity_manager::backup(Entity_ptr source) -> std::string {
		std::stringstream stream;
		auto serializer = EcsSerializer{stream, *this, _asset_mgr, {}};
		serializer.write(*source);
		stream.flush();

		return stream.str();
	}
	void Entity_manager::restore(Entity_ptr target, const std::string& data) {
		_entities.push_back(target);

		std::istringstream stream{data};
		auto deserializer = EcsDeserializer{"$EntityRestore", stream, *this, _asset_mgr, {}};
		deserializer.read(*target);
	}
	auto Entity_manager::restore(const std::string& data) -> Entity_ptr {
		auto target = emplace();

		std::istringstream stream{data};
		auto deserializer = EcsDeserializer{"$EntityRestore", stream, *this, _asset_mgr, {}};
		deserializer.read(*target);

		return target;
	}

	void Entity_manager::write(std::ostream& stream, Component_filter filter) {
		write(stream, _entities, filter);
	}

	void Entity_manager::write(std::ostream& stream,
	                           const std::vector<Entity_ptr>& entities,
	                           Component_filter filter) {

		auto serializer = EcsSerializer{stream, *this, _asset_mgr, filter};
		serializer.write_virtual(
			sf2::vmember("entities", entities)
		);

		stream.flush();

		DEBUG(entities.size()<<" saved. No survivors");
	}

	void Entity_manager::read(std::istream& stream, bool clear, Component_filter filter) {
		if(clear) {
			this->clear();
		}

		// read into dummy vector, because entity register themself when they are created
		std::vector<Entity_ptr> dummy;
		auto deserializer = EcsDeserializer{"$EntityDump", stream, *this, _asset_mgr, filter};
		deserializer.read_virtual(
			sf2::vmember("entities", dummy)
		);

		DEBUG("Loaded "<<dummy.size()<<" entities");
	}

	void Entity_manager::clear() {
		for(auto& cp : _pools)
			if(cp)
				cp->clear();

		_entities.clear();
		_delete_queue.clear();
	}

} /* namespace ecs */
}
