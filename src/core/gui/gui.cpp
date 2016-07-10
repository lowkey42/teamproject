#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION

#include "gui.hpp"

#include "../input/events.hpp"
#include "../input/input_manager.hpp"

#include <nuklear.h>
#include <SDL2/SDL.h>

#include <string>


namespace lux {
namespace gui {

	namespace {
		void nk_sdl_clipbard_paste(nk_handle, struct nk_text_edit *edit) {
			auto text = SDL_GetClipboardText();
			if (text) {
				nk_textedit_paste(edit, text, nk_strlen(text));
			}
		}

		void nk_sdl_clipbard_copy(nk_handle, const char *text, int len) {
			if (len<=0) return;

			auto str = std::string(text, static_cast<std::size_t>(len));
			SDL_SetClipboardText(str.c_str());
		}

		class Gui_event_filter : Sdl_event_filter {
			public:
				Gui_event_filter(Engine& e, nk_context* ctx)
					: _engine(e), _ctx(ctx) {

					_engine.add_event_filter(*this);
				}
				~Gui_event_filter() {
					_engine.remove_event_filter(*this);
				}

				bool propagate(SDL_Event&) {
					// TODO
					return true;
				}

			private:
				Engine& _engine;
				nk_context* _ctx;
		};

		struct Wnk_Context {
			nk_context ctx;

			Wnk_Context() {
				nk_init_default(&ctx, nullptr);
				ctx.clip.copy = nk_sdl_clipbard_copy;
				ctx.clip.paste = nk_sdl_clipbard_paste;
				ctx.clip.userdata = nk_handle_ptr(0);
			}
			~Wnk_Context() {
				nk_free(&ctx);
			}
		};
		struct Wnk_Buffer {
			nk_buffer buffer;

			Wnk_Buffer() {
				nk_buffer_init_default(&buffer);
			}
			~Wnk_Buffer() {
				nk_buffer_free(&buffer);
			}
		};

	}

	struct Gui::PImpl {
		Wnk_Context ctx;
		Wnk_Buffer buffer;
		// TODO: fonts
		Gui_event_filter input_filter;

		PImpl(Engine& e) : input_filter(e, &ctx.ctx) {}
	};

	Gui::Gui(Engine& engine)
	    : _impl(std::make_unique<PImpl>(engine)) {

		// TODO: rendering
	}

	void Gui::draw(renderer::Command_queue&) {
		// TODO
	}

}
}
