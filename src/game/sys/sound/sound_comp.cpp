#include "sound_comp.hpp"
#include "sound_sys.hpp"

namespace lux {
namespace sys {
namespace sound {

	class Sound_sys;


	Sound_comp::Sound_comp(ecs::Entity_manager& manager, ecs::Entity_handle owner, audio::Audio_ctx* ctx)
	    : Component(manager, owner), _ctx(ctx) {

		std::fill(_channels.begin(), _channels.end(), -1);
	}
	Sound_comp::Sound_comp(Sound_comp&& rhs)noexcept
	    : Component(std::move(rhs)), _ctx(rhs._ctx) {

		for(auto i=0u; i<max_channels; i++) {
			_channels[i] = rhs._channels[i];
			rhs._channels[i] = -1;
		}
	}
	Sound_comp& Sound_comp::operator=(Sound_comp&& rhs)noexcept {
		Component::operator=(std::move(rhs));

		if(!_ctx) {
			_ctx = rhs._ctx;
		}

		for(auto i=0u; i<max_channels; i++) {
			if(_channels[i]!=-1) {
				INVARIANT(_ctx, "Audio_ctx not set");
				_ctx->stop(_channels[i]);
			}
			_channels[i] = rhs._channels[i];
			rhs._channels[i] = -1;
		}

		return *this;
	}
	Sound_comp::~Sound_comp() {
		for(auto c : _channels) {
			if(c!=-1) {
				INVARIANT(_ctx, "Audio_ctx not set");
				_ctx->stop(c);
			}
		}
	}

}
}
}
