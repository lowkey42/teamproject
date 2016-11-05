#include "serializer.hpp"

#include "ecs.hpp"
#include "../asset/asset_manager.hpp"
#include "../utils/template_utils.hpp"

#include <sf2/sf2.hpp>

#include <unordered_map>
#include <vector>
#include <iostream>
#include <string>


namespace lux {
namespace ecs {
	using namespace asset;

	class Blueprint {
		public:
			Blueprint(std::string id, std::string content, asset::Asset_manager* asset_mgr);
			Blueprint(const Blueprint&) = delete;
			Blueprint(Blueprint&&) = default;
			~Blueprint()noexcept;
			Blueprint& operator=(Blueprint&&)noexcept;

			void detach(Entity_handle target)const;
			void on_reload();

			mutable std::vector<Entity_handle> users;
			mutable std::vector<Blueprint*> children;
			std::string id;
			std::string content;
			asset::Ptr<Blueprint> parent;
			asset::Asset_manager* asset_mgr;
			mutable Entity_manager* entity_manager;
	};
}

namespace asset {
	template<>
	struct Loader<ecs::Blueprint> {
		static auto load(istream in) throw(Loading_failed) -> std::shared_ptr<ecs::Blueprint> {
			return std::make_shared<ecs::Blueprint>(in.aid().str(), in.content(), &in.manager());
		}

		static void store(ostream out, ecs::Blueprint& asset) throw(Loading_failed) {
			FAIL("NOT IMPLEMENTED, YET!");
		}
	};
}

namespace ecs {
	class BlueprintComponent : public ecs::Component<BlueprintComponent> {
		public:
			static constexpr const char* name() {return "$Blueprint";}

			friend void load_component(ecs::Deserializer& state, BlueprintComponent&);
			friend void save_component(ecs::Serializer& state, const BlueprintComponent&);

			BlueprintComponent() = default;
			BlueprintComponent(ecs::Entity_manager& manager, ecs::Entity_handle owner,
			                   asset::Ptr<Blueprint> blueprint={})noexcept;
			BlueprintComponent(BlueprintComponent&&)noexcept = default;
			BlueprintComponent& operator=(BlueprintComponent&&) = default;
			~BlueprintComponent();

			void set(asset::Ptr<Blueprint> blueprint);

		private:
			asset::Ptr<Blueprint> blueprint;
	};
	Component_type blueprint_comp_id = component_type_id<BlueprintComponent>();

	namespace {
		sf2::format::Error_handler create_error_handler(std::string source_name) {
			return [source_name = std::move(source_name)](const std::string& msg, uint32_t row, uint32_t column) {
				ERROR("Error parsing JSON from "<<source_name<<" at "<<row<<":"<<column<<": "<<msg);
			};
		}
	}

	Deserializer::Deserializer(
	        const std::string& source_name, std::istream& stream,
	        Entity_manager& m, asset::Asset_manager& assets, Component_filter filter)
		: sf2::JsonDeserializer(
	          sf2::format::Json_reader{stream, create_error_handler(source_name)},
	          create_error_handler(source_name) ),
	      manager(m), assets(assets), filter(filter) {
	}

	namespace {
		const std::string import_key = "$import";

		void apply_blueprint(asset::Asset_manager& asset_mgr,
		                     Entity_facet e, const Blueprint& b) {
			if(b.parent) {
				apply_blueprint(asset_mgr, e, *b.parent);
			}

			std::istringstream stream{b.content};
			auto deserializer = Deserializer{b.id, stream, e.manager(), asset_mgr};

			auto handle = e.handle();
			deserializer.read_value(handle);
		}
	}


	void load_component(ecs::Deserializer& state, BlueprintComponent& comp) {
		std::string blueprintName;
		state.read_virtual(sf2::vmember("name", blueprintName));

		auto blueprint = state.assets.load<Blueprint>(AID{"blueprint"_strid, blueprintName});
		comp.set(blueprint);
		blueprint->users.push_back(comp.owner_handle());
		blueprint->entity_manager = &comp.manager();
		apply_blueprint(state.assets, comp.owner(), *blueprint);
	}
	void save_component(ecs::Serializer& state, const BlueprintComponent& comp) {
		auto name = comp.blueprint.aid().name();
		state.write_virtual(sf2::vmember("name", name));
	}

	BlueprintComponent::BlueprintComponent(ecs::Entity_manager& manager, ecs::Entity_handle owner,
	                                       asset::Ptr<Blueprint> blueprint)noexcept
	  : Component(manager,owner), blueprint(std::move(blueprint)) {

	}
	BlueprintComponent::~BlueprintComponent() {
		if(blueprint) {
			blueprint->detach(owner_handle());
			blueprint.reset();
		}
	}

	void BlueprintComponent::set(asset::Ptr<Blueprint> blueprint) {
		if(this->blueprint) {
			this->blueprint->detach(owner_handle());
		}

		this->blueprint = std::move(blueprint);
	}


	Blueprint::Blueprint(std::string id, std::string content, asset::Asset_manager* asset_mgr)
		: id(std::move(id)), content(std::move(content)), asset_mgr(asset_mgr) {

		std::istringstream stream{this->content};
		auto deserializer = sf2::JsonDeserializer{stream};
		deserializer.read_lambda([&](const auto& key) {
			if(key==import_key) {
				auto value = std::string{};
				deserializer.read_value(value);
				parent = asset_mgr->load<Blueprint>(AID{"blueprint"_strid, value});
				parent->children.push_back(this);

			} else {
				deserializer.skip_obj();
			}
			return true;
		});
	}
	Blueprint::~Blueprint()noexcept {
		if(parent) {
			util::erase_fast(parent->children, this);
		}
		INVARIANT(children.empty(), "Blueprint children not deregistered");
	}

	Blueprint& Blueprint::operator=(Blueprint&& o)noexcept {
		// swap data but keep user-list
		id = o.id;
		content = std::move(o.content);
		if(parent) {
			util::erase_fast(parent->children, this);
			parent.reset();
		}

		if(o.parent) {
			util::erase_fast(o.parent->children, &o);
			parent = std::move(o.parent);
			o.parent->children.push_back(this);
		}

		on_reload();

		return *this;
	}
	void Blueprint::on_reload() {
		for(auto&& c : children) {
			c->on_reload();
		}

		for(auto&& u : users) {
			auto entity = entity_manager->get(u);
			INVARIANT(entity.is_some(), "dead entity in blueprint.users");
			apply_blueprint(*asset_mgr, entity.get_or_throw(), *this);
		}
	}

	void Blueprint::detach(Entity_handle target)const {
		util::erase_fast(users, target);
	}

	void init_serializer(Entity_manager& ecs) {
		ecs.register_component_type<BlueprintComponent>();
	}

	void apply_blueprint(asset::Asset_manager& asset_mgr, Entity_facet e,
	                     const std::string& blueprint) {
		auto mb = asset_mgr.load_maybe<ecs::Blueprint>(asset::AID{"blueprint"_strid, blueprint});
		if(mb.is_nothing()) {
			ERROR("Failed to load blueprint \""<<blueprint<<"\"");
			return;
		}
		auto b = mb.get_or_throw();

		if(!e.has<BlueprintComponent>())
			e.emplace<BlueprintComponent>(b);
		else
			e.get<BlueprintComponent>().get_or_throw().set(b);

		b->users.push_back(e.handle());

		apply_blueprint(asset_mgr, e, *b);
	}

	void load(sf2::JsonDeserializer& s, Entity_handle& e) {
		auto& ecs_deserializer = static_cast<Deserializer&>(s);

		if(!ecs_deserializer.manager.validate(e)) {
			auto e_facet = ecs_deserializer.manager.emplace();
			e = e_facet.handle();
		}

		// TODO: static_assert(sf2::details::has_load<sf2::format::Json_reader,details::Component_base>::value, "missing load");
		s.read_lambda([&](const auto& key) {
			if(import_key==key) {
				auto value = std::string{};
				s.read_value(value);
				return true;
			}

			auto comp_type_mb = ecs_deserializer.manager.component_type_by_name(key);

			if(comp_type_mb.is_nothing()) {
				DEBUG("Skipped unknown component "<<key);
				s.skip_obj();
				return true;

			}

			auto comp_type = comp_type_mb.get_or_throw();

			if(ecs_deserializer.filter && !ecs_deserializer.filter(comp_type)) {
				DEBUG("Skipped filtered component "<<key);
				s.skip_obj();
				return true;
			}

			ecs_deserializer.manager.list(comp_type).restore(e, ecs_deserializer);

			return true;
		});
	}
	void save(sf2::JsonSerializer& s, const Entity_handle& e) {
		auto& ecs_s = static_cast<Serializer&>(s);

		s.write_lambda([&] {
			ecs_s.manager.list_all([&](auto& container) {
				if(!ecs_s.filter || ecs_s.filter(container.value_type())) {
					container.save(e, ecs_s);
				}
			});
		});
	}

}
}

