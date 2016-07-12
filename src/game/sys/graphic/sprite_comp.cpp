#define BUILD_SERIALIZER

#include "sprite_comp.hpp"

#include <core/utils/sf2_glm.hpp>

#include <sf2/sf2.hpp>

#include <string>

namespace lux {
namespace sys {
namespace graphic {

	using namespace unit_literals;

	void Sprite_comp::load(sf2::JsonDeserializer& state,
	                       asset::Asset_manager& assets){
		std::string aid = _material ? _material.aid().str() : "";
		auto hc_target_deg = _hue_change_target.in_degrees();
		auto hc_replacement_deg = _hue_change_replacement.in_degrees();

		state.read_virtual(
			sf2::vmember("material", aid),
			sf2::vmember("size", _size),
			sf2::vmember("shadowcaster", _shadowcaster),
			sf2::vmember("shadow_receiver", _shadow_receiver),
			sf2::vmember("hue_change_target", hc_target_deg),
			sf2::vmember("hue_change_replacement", hc_replacement_deg),
			sf2::vmember("decals_intensity", _decals_intensity)
		);

		_material = assets.load<renderer::Material>(asset::AID(aid));
		INVARIANT(_material, "Material '"<<aid<<"' not found");
		_hue_change_target = hc_target_deg * 1_deg;
		_hue_change_replacement = hc_replacement_deg * 1_deg;
	}

	void Sprite_comp::save(sf2::JsonSerializer& state)const {
		std::string aid = _material ? _material.aid().str() : "";

		state.write_virtual(
			sf2::vmember("material", aid),
			sf2::vmember("size", _size),
			sf2::vmember("shadowcaster", _shadowcaster),
			sf2::vmember("shadow_receiver", _shadow_receiver),
			sf2::vmember("hue_change_target", _hue_change_target.in_degrees()),
			sf2::vmember("hue_change_replacement", _hue_change_replacement.in_degrees()),
			sf2::vmember("decals_intensity", _decals_intensity)
		);
	}



	Anim_sprite_comp::Anim_sprite_comp(ecs::Entity& owner)
	    : Component(owner), _anim_state(&owner) {
	}

	void Anim_sprite_comp::load(sf2::JsonDeserializer& state,
	                       asset::Asset_manager& assets){
		std::string aid = _anim_state.animation_set() ? _anim_state.animation_set().aid().str() : "";
		auto anim_clip = _anim_state.get_clip();
		auto hc_target_deg = _hue_change_target.in_degrees();
		auto hc_replacement_deg = _hue_change_replacement.in_degrees();

		state.read_virtual(
			sf2::vmember("animation_set", aid),
			sf2::vmember("animation_clip", anim_clip),
			sf2::vmember("size", _size),
			sf2::vmember("shadowcaster", _shadowcaster),
			sf2::vmember("shadow_receiver", _shadow_receiver),
			sf2::vmember("hue_change_target", hc_target_deg),
			sf2::vmember("hue_change_replacement", hc_replacement_deg),
			sf2::vmember("decals_intensity", _decals_intensity)
		);

		auto anim_set = assets.load<renderer::Sprite_animation_set>(asset::AID(aid));
		INVARIANT(anim_set, "Animation_set '"<<aid<<"' not found");
		_anim_state.animation_set(anim_set, anim_clip);

		_hue_change_target = hc_target_deg * 1_deg;
		_hue_change_replacement = hc_replacement_deg * 1_deg;
	}

	void Anim_sprite_comp::save(sf2::JsonSerializer& state)const {
		std::string aid = _anim_state.animation_set() ? _anim_state.animation_set().aid().str() : "";

		state.write_virtual(
			sf2::vmember("animation_set", aid),
			sf2::vmember("animation_clip", _anim_state.get_clip()),
			sf2::vmember("size", _size),
			sf2::vmember("shadowcaster", _shadowcaster),
			sf2::vmember("shadow_receiver", _shadow_receiver),
			sf2::vmember("hue_change_target", _hue_change_target.in_degrees()),
			sf2::vmember("hue_change_replacement", _hue_change_replacement.in_degrees()),
			sf2::vmember("decals_intensity", _decals_intensity)
		);
	}

	void Anim_sprite_comp::play(renderer::Animation_clip_id id, float speed) {
		_anim_state.speed(speed);
		_anim_state.set_clip(id);
	}

	void Anim_sprite_comp::play_if(renderer::Animation_clip_id expected,
	             renderer::Animation_clip_id id, float speed) {
		if(!_anim_state.playing() || _anim_state.get_clip()==expected) {
			play(id, speed);
		}
	}

	void Anim_sprite_comp::play_next(renderer::Animation_clip_id id) {
		_anim_state.set_next_clip(id);
	}

	auto Anim_sprite_comp::playing()const noexcept -> util::maybe<renderer::Animation_clip_id> {
		return _anim_state.playing() ? util::just(_anim_state.get_clip())
		                             : util::nothing();
	}

}
}
}
