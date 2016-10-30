#include "ecs.hpp"

#include "component.hpp"
#include "serializer.hpp"

#include "../utils/string_utils.hpp"
#include "../utils/log.hpp"

#include "../engine.hpp"

#include <sf2/sf2.hpp>

#include <algorithm>
#include <stdexcept>
#include <iostream>



namespace lux {
namespace ecs {

	Entity_manager::Entity_manager(User_data& ud)
		: _userdata(ud) {

		// TODO: init_blueprints(*this);
	}

	Entity_facet Entity_manager::emplace()noexcept {
		return {*this, _handles.get_new()};
	}
	Entity_facet Entity_manager::emplace(const std::string& blueprint) {
		auto e = emplace();

		// TODO: apply_blueprint(_asset_mgr, *e, blueprint);

		return {*this, e.handle()};
	}

	auto Entity_manager::get(Entity_handle entity) -> util::maybe<Entity_facet> {
		if(validate(entity))
			return Entity_facet{*this, entity};

		return util::nothing();
	}

	void Entity_manager::erase(Entity_handle entity) {
		if(validate(entity)) {
			_queue_erase.enqueue(entity);
		} else {
			ERROR("Double-Deletion of entity "<<entity_name(entity));
		}
	}

	void Entity_manager::process_queued_actions() {
		INVARIANT(_local_queue_erase.empty(), "Someone's been sleeping in my bed! (_local_queue_erase is dirty)");

		std::array<Entity_handle, 32> erase_buffer;
		do {
			std::size_t count = _queue_erase.try_dequeue_bulk(erase_buffer.data(), erase_buffer.size());

			if(count>0) {
				for(std::size_t i=0; i<count; i++) {
					const auto h = erase_buffer[i];
					_local_queue_erase.emplace_back(h);

					for(auto& component : _components) {
						if(component)
							component->erase(h);
					}
				}

			} else {
				break;
			}
		} while(true);

		for(auto& component : _components) {
			if(component)
				component->process_queued_actions();
		}

		for(auto h : _local_queue_erase) {
			_handles.free(h);
		}
		_local_queue_erase.clear();
	}

	void Entity_manager::clear() {
		for(auto& component : _components)
			if(component)
				component->clear();

		_handles.clear();
		_queue_erase = Erase_queue{};
	}


	auto Entity_manager::write_one(Entity_handle source) -> ETO {
		/*
		std::stringstream stream;
		auto serializer = EcsSerializer{stream, *this, _userdata.assets(), {}};
		serializer.write(*source);
		stream.flush();

		return stream.str();
		*/
		return {};
	}
	auto Entity_manager::read_one(ETO data, Entity_handle target) -> Entity_facet {
		/*
		_entities.push_back(target);

		std::istringstream stream{data};
		auto deserializer = EcsDeserializer{"$EntityRestore", stream, *this, _userdata.assets(), {}};
		deserializer.read(*target);
	}
	auto Entity_manager::restore(const std::string& data) -> Entity_ptr {
		auto target = emplace();

		std::istringstream stream{data};
		auto deserializer = EcsDeserializer{"$EntityRestore", stream, *this, _userdata.assets(), {}};
		deserializer.read(*target);

		return target;
		*/
		return Entity_facet{};
	}

	void Entity_manager::write(std::ostream& stream, Component_filter filter) {
		// _handles.foreach_valid_handle([&](auto h){});

		// write(stream, _entities, filter);
	}

	void Entity_manager::write(std::ostream& stream,
	                           const std::vector<Entity_handle>& entities,
	                           Component_filter filter) {
/*
		auto serializer = EcsSerializer{stream, *this, _userdata.assets(), filter};
		serializer.write_virtual(
			sf2::vmember("entities", entities)
		);

		stream.flush();
*/
	}

	void Entity_manager::read(std::istream& stream, bool clear, Component_filter filter) {
/*
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
*/
	}
	
	
	Entity_collection_facet::Entity_collection_facet(Entity_manager& manager)
	    : _manager(manager) {
	}
	
	Entity_iterator Entity_collection_facet::begin()const {
		return Entity_iterator(_manager._handles, _manager._handles.next());
	}
	Entity_iterator Entity_collection_facet::end()const {
		return Entity_iterator(_manager._handles, invalid_entity);
	}
	void Entity_collection_facet::clear() {
		_manager.clear();
	}

} /* namespace ecs */
}
