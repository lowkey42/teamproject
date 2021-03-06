#include "serializer.hpp"

#include "../asset/asset_manager.hpp"
#include "ecs.hpp"
#include "../utils/template_utils.hpp"

#include <unordered_map>
#include <vector>
#include <sf2/sf2.hpp>
#include <iostream>


namespace lux {
namespace ecs {
	using namespace asset;
	using namespace sf2;
	using namespace sf2::io;

	class Blueprint {
		public:
			Blueprint(std::string id, std::string content, asset::Asset_manager* asset_mgr);
			Blueprint(const Blueprint&) = delete;
			Blueprint(Blueprint&&) = default;
			~Blueprint()noexcept;
			Blueprint& operator=(Blueprint&&)noexcept;

			void detach(Entity_ptr target)const;
			void on_reload();

			mutable std::vector<Entity*> users;
			mutable std::vector<Blueprint*> children;
			std::string id;
			std::string content;
			asset::Ptr<Blueprint> parent;
			asset::Asset_manager* asset_mgr;
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

			void load(sf2::JsonDeserializer& state,
			          asset::Asset_manager& asset_mgr)override;
			void save(sf2::JsonSerializer& state)const override;

			BlueprintComponent(ecs::Entity& owner, asset::Ptr<Blueprint> blueprint={})noexcept;
			BlueprintComponent(BlueprintComponent&&)noexcept = default;
			BlueprintComponent& operator=(BlueprintComponent&&) = default;
			~BlueprintComponent();

			void set(asset::Ptr<Blueprint> blueprint);

		private:
			asset::Ptr<Blueprint> blueprint;
	};
	Component_type blueprint_comp_id = BlueprintComponent::type();

	namespace {
		sf2::format::Error_handler create_error_handler(std::string source_name) {
			return [source_name = std::move(source_name)](const std::string& msg, uint32_t row, uint32_t column) {
				ERROR("Error parsing JSON from "<<source_name<<" at "<<row<<":"<<column<<": "<<msg);
			};
		}
	}

	EcsDeserializer::EcsDeserializer(
	        const std::string& source_name, std::istream& stream,
	        Entity_manager& m, asset::Asset_manager& assets, Component_filter filter)
		: sf2::JsonDeserializer(
	          format::Json_reader{stream, create_error_handler(source_name)},
	          create_error_handler(source_name) ),
	      manager(m), assets(assets), filter(filter) {
	}

	namespace {
		const std::string import_key = "$import";

		void apply_blueprint(asset::Asset_manager& asset_mgr,
		                     Entity& e, const Blueprint& b) {
			if(b.parent) {
				apply_blueprint(asset_mgr, e, *b.parent);
			}

			std::istringstream stream{b.content};
			auto deserializer = EcsDeserializer{b.id, stream, e.manager(), asset_mgr};

			deserializer.read_value(e);
		}
	}


	void BlueprintComponent::load(sf2::JsonDeserializer& state,
			  asset::Asset_manager& asset_mgr) {
		std::string blueprintName;
		state.read_virtual(sf2::vmember("name", blueprintName));

		blueprint = asset_mgr.load<Blueprint>(AID{"blueprint"_strid, blueprintName});
		this->set(blueprint);
		blueprint->users.push_back(&owner());
		apply_blueprint(asset_mgr, owner(), *blueprint);
	}
	void BlueprintComponent::save(sf2::JsonSerializer& state)const {
		auto name = blueprint.aid().name();
		state.write_virtual(sf2::vmember("name", name));
	}

	BlueprintComponent::BlueprintComponent(ecs::Entity& owner, asset::Ptr<Blueprint> blueprint)noexcept
	  : Component(owner), blueprint(std::move(blueprint)) {

	}
	BlueprintComponent::~BlueprintComponent() {
		if(blueprint) {
			blueprint->detach(owner_ptr());
			blueprint.reset();
		}
	}

	void BlueprintComponent::set(asset::Ptr<Blueprint> blueprint) {
		if(this->blueprint) {
			this->blueprint->detach(owner_ptr());
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

		for(auto&& u : users)
			apply_blueprint(*asset_mgr, *u, *this);
	}

	void Blueprint::detach(Entity_ptr target)const {
		util::erase_fast(users, target.get());
	}

	void init_blueprints(Entity_manager& ecs) {
		ecs.register_component_type<BlueprintComponent>();
	}

	void apply_blueprint(asset::Asset_manager& asset_mgr, Entity& e,
	                     asset::AID blueprint) {
		auto mb = asset_mgr.load_maybe<ecs::Blueprint>(blueprint);
		if(mb.is_nothing()) {
			ERROR("Failed to load blueprint \""<<blueprint.name()<<"\"");
			return;
		}
		auto b = mb.get_or_throw();

		if(!e.has<BlueprintComponent>())
			e.emplace<BlueprintComponent>(b);
		else
			e.get<BlueprintComponent>().get_or_throw().set(b);

		b->users.push_back(&e);

		apply_blueprint(asset_mgr, e, *b);
	}

	void load(sf2::JsonDeserializer& s, Entity& e) {
		auto& ecs_deserializer = static_cast<EcsDeserializer&>(s);

		static_assert(sf2::details::has_load<format::Json_reader,details::Component_base>::value, "missing load");
		s.read_lambda([&](const auto& key) {
			if(import_key==key) {
				auto value = std::string{};
				s.read_value(value);
				return true;
			}

			auto mb_comp = e.manager().find_comp_info(key);

			if(mb_comp.is_nothing()) {
				DEBUG("Skipped unknown component "<<key);
				s.skip_obj();
				return true;

			}

			auto& comp = mb_comp.get_or_throw();

			if(ecs_deserializer.filter && !ecs_deserializer.filter(comp.type)) {
				DEBUG("Skipped filtered component "<<key);
				s.skip_obj();
				return true;
			}

			auto comp_ptr = comp.get(e);
			if(!comp_ptr) {
				comp.add(e);
				comp_ptr = comp.get(e);
			}

			s.read_value(*comp_ptr);

			return true;
		});
	}
	void save(sf2::JsonSerializer& s, const Entity& e) {
		auto& ecs_s = static_cast<EcsSerializer&>(s);

		std::map<std::string, details::Component_base*> comps;

		EcsSerializer& ecss = static_cast<EcsSerializer&>(s);

		for(auto& comp : ecss.manager.list_comp_infos()) {
			const details::Component_type_info& compInfo = comp.second;

			if(ecs_s.filter && !ecs_s.filter(compInfo.type)) {
				continue;
			}

			auto comp_ptr = compInfo.get(const_cast<Entity&>(e));
			if(comp_ptr) {
				comps.emplace(comp.first, &*comp_ptr);
			}
		}

		s.write_value(comps);
	}
	void load(sf2::JsonDeserializer& s, Entity_ptr& e) {
		EcsDeserializer& ecss = static_cast<EcsDeserializer&>(s);

		e = ecss.manager.emplace();
		load(s, *e);
	}
}
}

