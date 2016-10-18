/** Entities that can play sound effects *************************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include <core/audio/audio_ctx.hpp>
#include <core/engine.hpp>
#include <core/units.hpp>
#include <core/ecs/component.hpp>


namespace lux {
namespace sys {
namespace sound {

	class Sound_sys;

	class Sound_comp : public ecs::Component<Sound_comp> {
		public:
			static constexpr const char* name() {return "Sound";}

			Sound_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner,
			           audio::Audio_ctx* ctx=nullptr);
			Sound_comp(Sound_comp&&)noexcept;
			Sound_comp& operator=(Sound_comp&&)noexcept;
			~Sound_comp();

		private:
			friend class Sound_sys;

			static constexpr auto max_channels = 4;

			audio::Audio_ctx* _ctx;
			std::array<audio::Channel_id, max_channels> _channels;
	};

}
}
}
