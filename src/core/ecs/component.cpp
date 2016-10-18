#include "component.hpp"

namespace lux {
namespace ecs {

	void Sparse_index_policy::attach(Entity_id owner, Component_index comp) {
		if(owner!=invalid_entity_id)
			_table.emplace(owner, comp);
	}
	void Sparse_index_policy::detach(Entity_id owner) {
		if(owner!=invalid_entity_id)
			_table.erase(owner);
	}
	void Sparse_index_policy::shrink_to_fit() {
	}
	auto Sparse_index_policy::find(Entity_id owner) -> util::maybe<Component_index> {
		if(owner==invalid_entity_id)
			return util::nothing();

		auto iter = _table.find(owner);
		if(iter!=_table.end()) {
			return iter->second;
		}

		return util::nothing();
	}
	void Sparse_index_policy::clear() {
		_table.clear();
	}


	void Compact_index_policy::attach(Entity_id owner, Component_index comp) {
		if(owner==invalid_entity_id)
			return;

		if(static_cast<Entity_id>(_table.size()) <= owner) {
			_table.resize(owner + 64);
		}

		_table[owner] = comp;
	}
	void Compact_index_policy::detach(Entity_id owner) {
		if(owner==invalid_entity_id)
			return;

		if(owner < static_cast<Entity_id>(_table.size())) {
			_table[owner] = -1;
		}
	}
	void Compact_index_policy::shrink_to_fit() {
		auto new_end = std::find(_table.rbegin(), _table.rend(), -1);
		_table.erase(std::next(new_end).base(), _table.end());
		_table.shrink_to_fit();
	}
	auto Compact_index_policy::find(Entity_id owner) -> util::maybe<Component_index> {
		if(owner==invalid_entity_id)
			return util::nothing();

		if(owner < static_cast<Entity_id>(_table.size())) {
			auto comp = _table[owner];
			if(comp!=-1) {
				return comp;
			}
		}

		return util::nothing();
	}
	void Compact_index_policy::clear() {
		_table.clear();
	}

}
}
