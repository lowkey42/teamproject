#define BUILD_SERIALIZER

#include "sound_sys.hpp"
#include "sound_comp.hpp"

#include <core/audio/sound.hpp>
#include <core/renderer/sprite_animation.hpp>
#include <core/utils/str_id.hpp>

#include <unordered_set>


namespace lux {
namespace sys {
namespace sound {

	namespace {
		struct Sound_effect_desc {
			std::string sound;
			bool loop = false;
		};

		sf2_structDef(Sound_effect_desc, sound, loop)
	}
	struct Sound_mappings {
		std::unordered_map<util::Str_id, Sound_effect_desc> event_sounds;
		std::unordered_set<util::Str_id> event_stop_loops;
		int rev = 0;

		Sound_mappings& operator=(Sound_mappings&& rhs) {
			event_sounds = std::move(rhs.event_sounds);
			event_stop_loops = std::move(rhs.event_stop_loops);
			rev = rhs.rev+1;
			return *this;
		}
	};
	sf2_structDef(Sound_mappings, event_sounds, event_stop_loops)

	Sound_sys::Sound_sys(Engine& engine)
	    : _audio_ctx(engine.audio_ctx()),
	      _assets(engine.assets()),
	      _mappings(engine.assets().load<Sound_mappings>("cfg:sound_effects"_aid)),
	      _mailbox(engine.bus()) {

		_reload();

		_mailbox.subscribe_to([&](const renderer::Animation_event& event) {
			_on_anim_event(event);
		});
	}

	Sound_sys::~Sound_sys() = default;

	void Sound_sys::update(Time dt) {
		if(_last_rev!=_mappings->rev) {
			_reload();
		}

		_mailbox.update_subscriptions();
	}

	void Sound_sys::_reload() {
		_event_sounds.clear();
		for(auto& e : _mappings->event_sounds) {
			auto aid = asset::AID{"sound"_strid, e.second.sound};
			_event_sounds[e.first] = Sound_effect{_assets.load<audio::Sound>(aid),
			                                           e.second.loop};
		}
	}
	void Sound_sys::_on_anim_event(const renderer::Animation_event& event) {
		if(!event.owner)
			return;

		if(_mappings->event_stop_loops.count(event.name)>0) {
			auto& e = *static_cast<ecs::Entity*>(event.owner);
			e.get<Sound_comp>().process([&](Sound_comp& s) {
				for(auto& c : s._channels) {
					_audio_ctx.stop(c);
					c=-1;
				}
			});
		}

		auto sound = _event_sounds.find(event.name);
		if(sound!=_event_sounds.end()) {

			if(!sound->second.loop) {
				_audio_ctx.play_static(*sound->second.sound);

			} else if(event.owner) {
				auto& e = *static_cast<ecs::Entity*>(event.owner);

				auto comp = e.get<Sound_comp>();
				if(comp.is_nothing()) {
					comp = util::justPtr(&e.emplace<Sound_comp>(&_audio_ctx));
				} else if(not comp.get_or_throw()._ctx) {
					comp.get_or_throw()._ctx = &_audio_ctx;
				}

				for(auto& channel : comp.get_or_throw()._channels) {
					if(!_audio_ctx.playing(channel)) {
						channel = _audio_ctx.play_static(*sound->second.sound, true);
						return;
					}
				}
				DEBUG("No free entity channel");
			}
		}
	}

}
}
}
