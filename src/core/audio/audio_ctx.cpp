#include "audio_ctx.hpp"

#include "sound.hpp"
#include "music.hpp"
#include "../asset/asset_manager.hpp"

#include <sf2/sf2.hpp>

#ifndef EMSCRIPTEN
#	include <SDL2/SDL_mixer.h>
#else
#	include <SDL/SDL_mixer.h>
#endif


namespace lux {
	namespace audio {
		namespace {

			struct Sounds_cfg {
				int frequence;
				int channels;
				int buffer_size;
				float music_volume;
				float sound_volume;
			};

			sf2_structDef(Sounds_cfg,
				frequence,
				channels,
				buffer_size,
				music_volume,
				sound_volume
			)

#ifndef EMSCRIPTEN
			constexpr auto default_cfg = Sounds_cfg{44100, 2, 4096, 0.8f, 1.0f};
#else
			constexpr auto default_cfg = Sounds_cfg{44100, 2, 4096, 0.8f, 1.0f};
#endif


			constexpr auto create_mask(int i) -> uint16_t {
				return i==1 ? 1 : (create_mask(i-1)<<1 | 1);
			}

			constexpr auto version_bits = 4u;

			constexpr auto version_mask = create_mask(version_bits);

			constexpr auto extract_channel(Channel_id c) {
				return static_cast<int16_t>(c>>version_bits);
			}
			constexpr auto extract_version(Channel_id c) {
				return static_cast<uint8_t>(c & version_mask);
			}
			constexpr auto build_channel_id(int16_t channel, uint8_t version) {
				return channel<<version_bits | (version & version_mask);
			}
		}

		using namespace unit_literals;

		Audio_ctx::Audio_ctx(asset::Asset_manager& assets) {
			_channels.fill(false);
			_channels_last.fill(false);
			_channel_versions.fill(0);
			_channel_sounds.fill(nullptr);
			_free_channels.reserve(_dynamic_channels);
			for(int16_t i=0; i<_dynamic_channels; ++i)
				_free_channels.push_back(i);


			auto& cfg = asset::unpack(assets.load_maybe<Sounds_cfg>("cfg:sounds"_aid)).get_or_other(
				 default_cfg
			);

			if(&cfg==&default_cfg) {
				assets.save<Sounds_cfg>("cfg:sounds"_aid, cfg);
			}

			// Checking if frequence is faulty -> setting to 44100MHz
			int verified_frequence = (cfg.frequence % 22050 == 0) ? cfg.frequence : 44100;
			DEBUG("frequence: " << verified_frequence << " | " << ((cfg.channels == 1) ? "Mono" : "Stereo")
								<< " | buffer: " << cfg.buffer_size);

			int mix_flags = MIX_INIT_OGG;
			if((Mix_Init(mix_flags)&mix_flags) != mix_flags) {
				FAIL("Initializing Mixer failed: " << Mix_GetError());
			}

			// Open SDL Audio Mixer
			if(Mix_OpenAudio(verified_frequence, MIX_DEFAULT_FORMAT, cfg.channels, cfg.buffer_size) == 0) {
				DEBUG("Sound_ctx succesfully initialized!");
			} else {
				FAIL("Initializing Sound incomplete: " << Mix_GetError());
			}

			Mix_AllocateChannels(_dynamic_channels+_static_channels);
			Mix_ReserveChannels(_dynamic_channels);

			sound_volume(cfg.sound_volume);
			music_volume(cfg.music_volume);
		}

		Audio_ctx::~Audio_ctx() {
			Mix_CloseAudio();
			Mix_Quit();
		}

		auto Audio_ctx::_reserve_channel(int16_t channel) -> uint8_t {
			auto& v = _channel_versions.at(channel);
			v = (v+1) & version_mask;
			return v;
		}

		void Audio_ctx::flip() {
			for(int16_t i=0; i<_dynamic_channels; ++i) {
				if(!_channels[i] && _channels_last[i]) {
					Mix_Pause(i);
					_free_channels.push_back(i);
				}
			}

			_channels_last = _channels;
			_channels.fill(false);
		}

		void Audio_ctx::play_music(std::shared_ptr<const audio::Music> music, Time fade_time) {
			if(_playing_music==music)
				return;

			if(_playing_music) {
				if(!music)
					Mix_FadeOutMusic(static_cast<int>(fade_time/1_ms));
				else
					Mix_HaltMusic(); // no cross-fade in SDL_Mixer, sadly
			}

			_playing_music = music;

			if(!music || !music->valid())
				return;

#ifndef EMSCRIPTEN
			Mix_FadeInMusic(music->getMusic(), -1, static_cast<int>(fade_time/1_ms));
#else
			Mix_PlayMusic(music->getMusic(), -1);
#endif
		}

		auto Audio_ctx::play_static(const audio::Sound& sound, bool loop) -> Channel_id {
			if(!sound.valid())
				return -1;

			auto c = Mix_PlayChannel(-1, sound.getSound(), loop ? -1 : 0);

			return build_channel_id(c, _reserve_channel(c));
		}
		auto Audio_ctx::play_dynamic(const audio::Sound& sound, Angle angle, float dist_percentage,
		                             bool loop, Channel_id id) -> Channel_id {
			if(!sound.valid())
				return -1;

			int16_t c;
			uint8_t v;

			if(id>=0) {
				c = extract_channel(id);
				v = extract_version(id);

				if(v!=_channel_versions[c])
					id=-1;

				else if(_channels[c]==_channels_last[c] && !_channels[c])
					id=-1;

				else if(&sound == _channel_sounds[c]) {
					if(Mix_Paused(c)) {
						Mix_Resume(c);
						update(id, angle, dist_percentage);
						return id;

					} else if(Mix_Playing(c)) {
						update(id, angle, dist_percentage);
						return id;
					}
				}
			}

			if(id<0) {
				if(_free_channels.empty()) {
					DEBUG("No free channel left.");
					return -1;
				}

				c = _free_channels.back();
				_free_channels.pop_back();

				v = _reserve_channel(c);

				id = build_channel_id(c,v);
			}

			_channel_sounds[c] = &sound;
			Mix_PlayChannel(c, sound.getSound(), loop ? -1 : 0);
			update(id, angle, dist_percentage);
			return id;
		}

		void Audio_ctx::update(Channel_id id, Angle angle, float dist_percentage) {
			if(id<0) return;

			auto c = extract_channel(id);
			auto v = extract_version(id);
			if(_channel_versions[c]==v) {
#ifndef EMSCRIPTEN
				auto a = angle.in_degrees();

				// Converting Coordinate System from game (up = 90, right = 180 / -180, down = -90, left = 0)
				//								to Mixer  (up = 0, right = 90, down = 180, left = 270)

				// new System is now (up = 0, right = 90 / -270, down = -180, left = -90)
				a -= 90.f;

				// Checking if value is < 0, if so calculate 360° - absolute of a
				// new System ist now (up = 0, right = 90, down = 180, left = 270 --> Mixer System)

				if(a - 90.f < 0){
					a = 360.f - std::abs(a);
				}

				auto dist = dist_percentage *255;
				if(!Mix_SetPosition(c, a, dist)) {
					DEBUG("Mix_SetPosition failed");
				}
#else
				Mix_Volume(c, std::max(1-dist_percentage, 0.f) * 255 * _sound_volume);
#endif
				if(c<_dynamic_channels) {
					_channels[c] = true;
				}
			}
		}

		bool Audio_ctx::playing(Channel_id id) {
			if(id<0) return false;

			auto c = extract_channel(id);
			auto v = extract_version(id);

			return _channel_versions[c]==v && Mix_Playing(c)==1;
		}

		void Audio_ctx::stop(Channel_id id) {
			if(id<0) return;

			auto c = extract_channel(id);
			auto v = extract_version(id);
			if(_channel_versions[c]==v) {
#ifndef EMSCRIPTEN
				Mix_FadeOutChannel(c, 100);
#else
				Mix_HaltChannel(c);
#endif
				if(c<_dynamic_channels) {
					_free_channels.push_back(c);
					_channels[c]=_channels_last[c] = false;
					_channel_sounds[c] = nullptr;
				}
			}
		}

		void Audio_ctx::pause_sounds() {
			Mix_Pause(-1);
		}

		void Audio_ctx::resume_sounds() {
			Mix_Resume(-1);
		}

		void Audio_ctx::stop_sounds() {
			Mix_HaltChannel(-1);
			_channels.fill(false);
			_channels_last.fill(false);
			_channel_versions.fill(0);
			_channel_sounds.fill(nullptr);

			_free_channels.clear();
			for(int i=0; i<_dynamic_channels; ++i)
				_free_channels.push_back(i);
		}

		void Audio_ctx::sound_volume(float volume) {
			_sound_volume = volume;
			Mix_Volume(-1, 128 * _sound_volume);
		}

		void Audio_ctx::music_volume(float volume) {
			_music_volume = volume;
			Mix_VolumeMusic(128 * _music_volume);
		}

	}
}

