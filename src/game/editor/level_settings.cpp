#include "level_settings.hpp"

#include <core/gui/gui.hpp>
#include <core/gui/translations.hpp>


namespace lux {
namespace editor {

	struct Level_settings::PImpl {
		std::string last_level_id;
		gui::Text_edit input_name;
		gui::Text_edit input_desc;
		gui::Text_edit input_author;
	};

	Level_settings::Level_settings(gui::Gui& gui, const gui::Translator& translator,
	                               Meta_system& systems)
	    : _impl(std::make_unique<PImpl>()),
	      _gui(gui),
	      _translator(translator),
	      _systems(systems) {
	}
	Level_settings::~Level_settings() = default;

	void Level_settings::update_and_draw(Level_info& metadata) {

		if(_impl->last_level_id!=metadata.id) {
			_impl->last_level_id = metadata.id;
			_impl->input_name.reset(metadata.name);
			_impl->input_desc.reset(metadata.description);
			_impl->input_author.reset(metadata.author);
		}

		// TODO: use translator
		// TODO: allow for undo/redo

		struct nk_panel layout;
		auto ctx = _gui.ctx();
		if (nk_begin(ctx, &layout, "Settings", nk_rect(0, 0, 400, 800),
		             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|
		             NK_WINDOW_TITLE|NK_WINDOW_SCROLL_AUTO_HIDE)) {

			constexpr auto row_h = 25;

			nk_layout_row_dynamic(ctx, row_h, 1);

			nk_label(ctx, "Name:", NK_TEXT_LEFT);
			_impl->input_name.update_and_draw(ctx, NK_EDIT_FIELD, metadata.name);

			nk_label(ctx, "Author:", NK_TEXT_LEFT);
			_impl->input_author.update_and_draw(ctx, NK_EDIT_FIELD, metadata.author);

			nk_label(ctx, "Description:", NK_TEXT_LEFT);
			nk_layout_row_dynamic(ctx, _impl->input_desc.active() ? 120 : 50, 1);
			_impl->input_desc.update_and_draw(ctx, NK_EDIT_BOX, metadata.description);
			nk_layout_row_dynamic(ctx, row_h, 1);

			nk_layout_row_dynamic(ctx, row_h*2, 1);
			nk_label(ctx, "Ambient light:", NK_TEXT_LEFT);
			nk_layout_row_dynamic(ctx, row_h, 1);

			metadata.ambient_brightness = nk_propertyf(ctx, "Intensity:", 0,
			                                           metadata.ambient_brightness,
			                                           2.0, 0.04f,0.01f);

			nk_layout_row_dynamic(ctx, row_h*2, 1);
			nk_label(ctx, "Sun:", NK_TEXT_LEFT);
			nk_layout_row_dynamic(ctx, row_h, 1);

			gui::color_picker(ctx, metadata.environment_light_color, 400, 10.f);
			nk_layout_row_dynamic(ctx, row_h, 1);

			auto& dir = metadata.environment_light_direction;
			auto nx = nk_propertyf(ctx, "X:", -1.0, dir.x, 1.0, 0.04f,0.01f);
			auto ny = nk_propertyf(ctx, "Y:", -1.0, dir.y, 1.0, 0.04f,0.01f);
			auto nz = nk_propertyf(ctx, "Z:", -1.0, dir.z, 1.0, 0.04f,0.01f);
			bool mod_x = std::abs(nx-dir.x)>0.0001f; dir.x = nx;
			bool mod_y = std::abs(ny-dir.y)>0.0001f; dir.y = ny;
			bool mod_z = std::abs(nz-dir.z)>0.0001f; dir.z = nz;
			auto dir_len = glm::length(dir);
			dir = dir_len>0.0001f ? dir/dir_len : glm::vec3(0,-0.9f,-0.43f);
			if(mod_x) {
				dir.x = nx;
			} else if(mod_y) {
				dir.y = ny;
			} else if(mod_z) {
				dir.z = nz;
			}

			nk_layout_row_dynamic(ctx, row_h*2, 1);

			nk_label(ctx, "Background:", NK_TEXT_LEFT);
			nk_layout_row_dynamic(ctx, row_h, 1);
			// TODO: selection of image
			metadata.environment_brightness = nk_propertyf(ctx, "Brightness:", 0,
			                                               metadata.environment_brightness,
			                                               1.0, 0.02f,0.005f);
			gui::color_picker(ctx, metadata.background_tint, 400);
			nk_layout_row_dynamic(ctx, row_h, 1);

			nk_layout_row_dynamic(ctx, row_h*2, 1);

			// TODO: selection of music
			nk_layout_row_dynamic(ctx, row_h, 1);


			_systems.light_config(metadata.environment_light_color,
			                      metadata.environment_light_direction,
			                      metadata.ambient_brightness,
			                      metadata.background_tint,
			                      metadata.environment_brightness);

		} else {
			_visible = false;
		}
		nk_window_show(ctx, "Settings", _visible ? NK_SHOWN : NK_HIDDEN);
		nk_end(ctx);
	}

}
}
