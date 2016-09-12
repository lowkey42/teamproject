/** Sound & Music context creation + management ******************************
 *                                                                           *
 * Copyright (c) 2015 Florian Oetke & Sebastian Schalow                      *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <array>

#include "../units.hpp"

namespace lux {
	namespace asset {
		class Asset_manager;
	}

namespace audio{

	class Music;
	class Sound;

	using Channel_id = int16_t;

	class Audio_ctx {
		public:
			Audio_ctx(asset::Asset_manager& assets);
			~Audio_ctx();

			void flip();

			void play_music  (std::shared_ptr<const audio::Music> music, Time fade_time = Time{0});
			auto play_static (const audio::Sound& sound, bool loop=false) -> Channel_id;
			auto play_dynamic(const audio::Sound& sound, Angle angle, float dist_percentage,
			                  bool loop, Channel_id=-1) -> Channel_id;

			void update    (Channel_id id, Angle angle, float dist_percentage);
			void stop      (Channel_id);
			bool playing   (Channel_id);
			void stop_music(Time fade_time=Time{0}) {play_music({}, fade_time);}

			void pause_sounds ();
			void resume_sounds();
			void stop_sounds  ();

			void sound_volume(float volume);
			void music_volume(float volume);
			auto sound_volume()const noexcept {return _sound_volume;}
			auto music_volume()const noexcept {return _music_volume;}

		private:
			static constexpr int16_t _dynamic_channels = 128;
			static constexpr int16_t _static_channels = 128;
			using sptr = const audio::Sound*;

			float _sound_volume;
			float _music_volume;

			std::array<bool,    _dynamic_channels> _channels;
			std::array<bool,    _dynamic_channels> _channels_last;
			std::array<sptr,    _dynamic_channels> _channel_sounds;
			std::vector<int16_t>                   _free_channels;

			std::array<uint8_t, _dynamic_channels+_static_channels> _channel_versions;

			std::shared_ptr<const audio::Music>    _playing_music;

			auto _reserve_channel(int16_t channel) -> uint8_t;
	};

}
}

