#include "level_settings.hpp"

#include <core/gui/gui.hpp>
#include <core/gui/translations.hpp>


namespace lux {
namespace editor {

	// TODO: everything
	// TODO: use HSV for color-pickers
	bool draw_settings(const gui::Translator&, gui::Gui& gui, bool visible,
	                   Level_info& metadata, Meta_system& sys) {

		struct nk_panel layout;
		auto ctx = gui.ctx();
		if (nk_begin(ctx, &layout, "Settings Ä b", nk_rect(0, 0, 210, 250),
		             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|
		             NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {

			static char name[50] {};
			static int name_len = 0;
			nk_layout_row_dynamic(ctx, 60, 1);
			nk_edit_string(ctx, NK_EDIT_FIELD, name, &name_len, 50, nk_filter_default);

			nk_color ambient_light = nk_rgb(metadata.environment_light_color.r*255 / 10.f,
			                                 metadata.environment_light_color.g*255 / 10.f,
			                                 metadata.environment_light_color.b*255 / 10.f);
			nk_panel combo;
			nk_layout_row_dynamic(ctx, 20, 1);
			nk_label(ctx, "Sun color:", NK_TEXT_LEFT);
			nk_layout_row_dynamic(ctx, 25, 1);
			if (nk_combo_begin_color(ctx, &combo, ambient_light, 400)) {
				nk_layout_row_dynamic(ctx, 120, 1);
				ambient_light = nk_color_picker(ctx, ambient_light, NK_RGB);
				nk_layout_row_dynamic(ctx, 25, 1);
				ambient_light.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, ambient_light.r, 255, 1,1);
				ambient_light.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, ambient_light.g, 255, 1,1);
				ambient_light.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, ambient_light.b, 255, 1,1);
				// ambient_light.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, ambient_light.a, 255, 1,1);

				metadata.environment_light_color = Rgb{
					ambient_light.r/255.f * 10.f,
					ambient_light.g/255.f * 10.f,
					ambient_light.b/255.f * 10.f
				};

				sys.light_config(metadata.environment_light_color,
				                 metadata.environment_light_direction,
				                 metadata.ambient_brightness,
				                 metadata.background_tint);

				nk_combo_end(ctx);
			}

		} else {
			visible = false;
		}
		nk_window_show(ctx, "Settings", visible ? NK_SHOWN : NK_HIDDEN);
		nk_end(ctx);

		return visible;
	}

}
}
