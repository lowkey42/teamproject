#define GLM_SWIZZLE

#include "selection.hpp"

#include "editor_comp.hpp"

#include "../physics/transform_comp.hpp"
#include "../graphic/graphic_system.hpp"

#include <core/input/input_manager.hpp>
#include <core/renderer/primitives.hpp>
#include <core/renderer/command_queue.hpp>

#include <glm/gtx/vector_angle.hpp>


namespace mo {
namespace sys {
namespace editor {

	using namespace unit_literals;
	using namespace renderer;
	using namespace glm;

	namespace {
		struct Selection_change_cmd : util::Command {
			public:
				Selection_change_cmd(Selection& mgr, ecs::Entity_ptr e)
				    : _selection_mgr(mgr), _selection(e) {}

				void execute()override {
					_prev_selection = _selection_mgr.selection();
					_selection_mgr.select(_selection);
				}
				void undo()override {
					_selection_mgr.select(_prev_selection);
				}
				auto name()const -> const std::string& override{
					return _name;
				}

			private:
				static const std::string _name;
				Selection& _selection_mgr;

				ecs::Entity_ptr _selection;
				ecs::Entity_ptr _prev_selection;
		};
		const std::string Selection_change_cmd::_name = "Selection changed";

		struct Transform_cmd : util::Command {
			public:
				Transform_cmd(ecs::Entity_ptr e, Position new_pos, Angle new_rot, float new_scale,
				              Position prev_pos, Angle prev_rot, float prev_scale)
				    : _entity(e),
				      _new_position(new_pos), _new_rotation(new_rot), _new_scale(new_scale),
				      _prev_position(prev_pos), _prev_rotation(prev_rot), _prev_scale(prev_scale) {}

				void execute()override {
					auto& transform = _entity->get<physics::Transform_comp>().get_or_throw();

					transform.position(_new_position);
					transform.rotation(_new_rotation);
					transform.scale(_new_scale);
				}
				void undo()override {
					auto& transform = _entity->get<physics::Transform_comp>().get_or_throw();

					transform.position(_prev_position);
					transform.rotation(_prev_rotation);
					transform.scale(_prev_scale);
				}
				auto name()const -> const std::string& override{
					return _name;
				}

			private:
				static const std::string _name;

				ecs::Entity_ptr _entity;

				Position  _new_position;
				Angle     _new_rotation;
				float     _new_scale;

				Position  _prev_position;
				Angle     _prev_rotation;
				float     _prev_scale;
		};
		const std::string Transform_cmd::_name = "Entity transformed";


		bool is_inside(ecs::Entity& e, glm::vec2 p, Camera& cam, bool forgiving=false) {
			bool inside = false;

			// TODO: check for smart_texture

			process(e.get<physics::Transform_comp>(), e.get<Editor_comp>())
			        >> [&](auto& transform, auto& editor) {
				auto center = remove_units(transform.position());
				auto half_bounds = remove_units(editor.bounds()).xy() * transform.scale() / 2.f;
				if(forgiving)
					half_bounds = glm::max(half_bounds, vec2{0.5f,0.5f}) + 0.5f;

				auto world_p = cam.screen_to_world(p, center).xy();
				auto obb_p = rotate(world_p - center.xy(), -transform.rotation());

				inside = obb_p.x > -half_bounds.x && obb_p.x < half_bounds.x &&
				         obb_p.y > -half_bounds.y && obb_p.y < half_bounds.y;
			};

			return inside;
		}
	}

	Selection::Selection(Engine& engine, ecs::Entity_manager& entity_manager,
	                     renderer::Camera& world_cam, util::Command_manager& commands)
	    : _mailbox(engine.bus()), _world_cam(world_cam), _commands(commands),
	      _input_manager(engine.input()),
	      _editor_comps(entity_manager.list<Editor_comp>()),
	      _last_primary_pointer_pos(util::nothing()),
	      _last_secondary_pointer_pos(util::nothing()) {

		_icon_layer  = engine.assets().load<Texture>("tex:selection_icon_layer"_aid);
		_icon_move   = engine.assets().load<Texture>("tex:selection_icon_move"_aid);
		_icon_rotate = engine.assets().load<Texture>("tex:selection_icon_rotate"_aid);
		_icon_scale  = engine.assets().load<Texture>("tex:selection_icon_scale"_aid);


		_mailbox.subscribe_to([&](input::Continuous_action& e) {
			switch(e.id) {
				case "mouse_down"_strid:
					if(e.begin) {
						if(_selected_entity) {
							_current_action = Action_type::none;
							auto& transform = _selected_entity->get<physics::Transform_comp>().get_or_throw();

							_prev_entity_position = remove_units(transform.position());
							_prev_entity_rotation = transform.rotation();
							_prev_entity_scale = transform.scale();

							_curr_entity_position = _prev_entity_position;
							_curr_entity_rotation = _prev_entity_rotation;
							_curr_entity_scale    = _prev_entity_scale;
						}
					} else {
						if(_selected_entity && _current_action!=Action_type::none) {
							DEBUG("Commit entity transformations");
							auto& transform = _selected_entity->get<physics::Transform_comp>().get_or_throw();
							_commands.execute<Transform_cmd>(_selected_entity,
							                                 transform.position(),
							                                 transform.rotation(),
							                                 transform.scale(),
							                                 _prev_entity_position*1_m,
							                                 _prev_entity_rotation,
							                                 _prev_entity_scale);
						}

						_current_action = Action_type::none;
					}
					break;
			}

		});
		_mailbox.subscribe_to([&](input::Once_action& e) {
			switch(e.id) {
				case "mouse_click"_strid: {
					auto mp = _input_manager.last_pointer_screen_position();
					_change_selection(mp);
					break;
				}
			}
		});
	}

	void Selection::draw(renderer::Command_queue& queue, renderer::Camera& cam) {
		if(_selected_entity) {
			auto& transform = _selected_entity->get<physics::Transform_comp>().get_or_throw();
			auto editor = _selected_entity->get<Editor_comp>();

			if(false /* TODO: check if this is a smart_texture */) {
				// TODO: draw edges and nodes

			} else if(editor.is_some()) {
				auto center = remove_units(transform.position());
				auto bounds = remove_units(editor.get_or_throw().bounds()).xy() * transform.scale();
				bounds = glm::max(bounds, vec2{1,1});

				auto top_left     = vec2{-bounds.x/2.f,  bounds.y/2.f};
				auto bottom_right = top_left*-1.f;
				auto top_right    = glm::vec2{bottom_right.x, top_left.y};
				auto bottom_left  = glm::vec2{top_left.x, bottom_right.y};

				top_left     = rotate(top_left, transform.rotation())     + center.xy();
				bottom_right = rotate(bottom_right, transform.rotation()) + center.xy();
				top_right    = rotate(top_right, transform.rotation())    + center.xy();
				bottom_left  = rotate(bottom_left, transform.rotation())  + center.xy();

				// world to screen
				top_left     = _world_cam.world_to_screen(vec3(top_left, center.z));
				bottom_right = _world_cam.world_to_screen(vec3(bottom_right, center.z));
				top_right    = _world_cam.world_to_screen(vec3(top_right, center.z));
				bottom_left  = _world_cam.world_to_screen(vec3(bottom_left, center.z));

				// screen to menu
				top_left     = cam.screen_to_world(top_left).xy();
				bottom_right = cam.screen_to_world(bottom_right).xy();
				top_right    = cam.screen_to_world(top_right).xy();
				bottom_left  = cam.screen_to_world(bottom_left).xy();

				draw_dashed_line(queue, top_left,     top_right,    2.0f, Rgba{1.f,0.8f, 0.25f, 1.0f});
				draw_dashed_line(queue, top_right,    bottom_right, 2.0f, Rgba{1.f,0.8f, 0.25f, 1.0f});
				draw_dashed_line(queue, bottom_right, bottom_left,  2.0f, Rgba{1.f,0.8f, 0.25f, 1.0f});
				draw_dashed_line(queue, bottom_left,  top_left,     2.0f, Rgba{1.f,0.8f, 0.25f, 1.0f});

				_batch.insert(*_icon_layer, top_left, {32.f, 32.f});
				_batch.insert(*_icon_rotate, bottom_right, {32.f, 32.f});
				_batch.insert(*_icon_scale, top_right, {32.f, 32.f});
				_batch.flush(queue);
			}
		}
	}
	void Selection::update() {
		_mailbox.update_subscriptions();
	}

	auto Selection::handle_pointer(const util::maybe<glm::vec2> mp1,
	                               const util::maybe<glm::vec2> mp2) -> bool {
		ON_EXIT {
			_last_primary_pointer_pos = mp1;
			_last_secondary_pointer_pos = mp2;
		};

		if(mp1.is_nothing() || _last_primary_pointer_pos.is_nothing() || !_selected_entity)
			return false;

		if(_current_action==Action_type::none && !is_inside(*_selected_entity, _last_primary_pointer_pos.get_or_throw(), _world_cam, true))
			return false;


		if(mp2.is_some())
			return _handle_multitouch(mp1.get_or_throw(), mp2.get_or_throw());
		else
			return _handle_singletouch(_last_primary_pointer_pos.get_or_throw(), mp1.get_or_throw());
	}

	auto Selection::_handle_multitouch(glm::vec2 mp1_curr, glm::vec2 mp2_curr) -> bool {
		auto mp1_prev = _last_primary_pointer_pos.get_or_other(mp1_curr);
		auto mp2_prev = _last_secondary_pointer_pos.get_or_other(mp2_curr);

		auto mp1_delta = mp1_curr - mp1_prev;
		_move(mp1_delta);


		auto offset_curr = mp2_curr - mp1_curr;
		auto offset_prev = mp2_prev - mp1_prev;

		auto offset_curr_len = glm::length(offset_curr);
		auto offset_prev_len = glm::length(offset_prev);

		if(offset_prev_len>0.f && offset_curr_len>0.f) {
			_scale(mp1_curr, offset_curr_len / offset_prev_len);

			auto angle_curr = Angle{glm::atan(offset_curr.y, offset_curr.x)};
			auto angle_prev = Angle{glm::atan(offset_prev.y, offset_prev.x)};
			_rotate(mp1_curr, angle_prev - angle_curr);
		}

		return true;
	}
	auto Selection::_handle_singletouch(glm::vec2 prev, glm::vec2 curr) -> bool {
		auto& transform = _selected_entity->get<physics::Transform_comp>().get_or_throw();
		auto& editor    = _selected_entity->get<Editor_comp>().get_or_throw();


		if(false /* TODO: check if this is a smart_texture */) {
			if(false /* TODO: check if prev is inside of a marker or near a border */) {
				// ...
				return true;
			}
			_move(curr - prev);
			return true;

		}

		auto center      = remove_units(transform.position());
		auto half_bounds = glm::max(remove_units(editor.bounds()).xy() * transform.scale(), vec2{1,1}) / 2.f;

		auto icon_layer_pos = vec2{-half_bounds.x,  half_bounds.y};
		auto icon_rotate_pos = vec2{half_bounds.x, -half_bounds.y};
		auto icon_scale_pos = vec2{half_bounds.x, half_bounds.y};

		auto world_prev = _world_cam.screen_to_world(prev, center).xy();
		auto obb_prev = rotate(world_prev - center.xy(), -transform.rotation());

		auto world_curr = _world_cam.screen_to_world(curr, center).xy();
		auto obb_curr = rotate(world_curr - center.xy(), -transform.rotation());

		auto icon_radius = glm::length(_world_cam.screen_to_world(vec2{16.f, 16.f}, center).xy() - _world_cam.screen_to_world(vec2{0.f, 0.f}, center).xy());
		auto icon_size2 = icon_radius*icon_radius;

		if(_current_action==Action_type::none) {
			if(glm::length2(obb_prev-icon_layer_pos)<icon_size2) {
				_current_action = Action_type::layer;
			} else if(glm::length2(obb_prev-icon_rotate_pos)<icon_size2) {
				_current_action = Action_type::rotate;
			} else if(glm::length2(obb_prev-icon_scale_pos)<icon_size2) {
				_current_action = Action_type::scale;
			} else {
				_current_action = Action_type::move;
			}
		}

		switch(_current_action) {
			case Action_type::move:
				_move(curr - prev);
				break;

			case Action_type::scale:
				_scale(glm::length(obb_curr) / glm::length(obb_prev));
				break;

			case Action_type::rotate: {
				auto angle_curr = Angle{glm::atan(obb_curr.y, obb_curr.x)};
				auto angle_prev = Angle{glm::atan(obb_prev.y, obb_prev.x)};
				_rotate(angle_curr - angle_prev);
				break;
			}

			case Action_type::layer:
				_move_layer((curr.y - prev.y) * 0.01);
				break;

			case Action_type::none:
				FAIL("UNREACHABLE: Couldn't determine action_type");
				break;
		}

		return true;
	}

	auto Selection::copy_content()const -> std::string {
		// TODO
		return "[]";
	}

	void Selection::_change_selection(glm::vec2 point) {
		ecs::Entity_ptr entity;
		float max_z = -1000.f;

		for(auto& comp : _editor_comps) {
			if(is_inside(comp.owner(), point, _world_cam)) {
				auto z = comp.owner().get<physics::Transform_comp>().get_or_throw().position().z.value();
				if(z>=max_z) {
					max_z = z;
					entity = comp.owner_ptr();
				}
			}
		}

		_commands.execute<Selection_change_cmd>(*this, entity);
	}

	void Selection::_move(glm::vec2 offset) {
		if(!_selected_entity || glm::length2(offset)<1.f)
			return;

		auto world_offset = _world_cam.screen_to_world(offset, _curr_entity_position)
		                    - _world_cam.screen_to_world(vec2{0,0}, _curr_entity_position);

		_curr_entity_position += vec3(world_offset.xy(), 0.0f);
		_update_entity_transform();
	}
	void Selection::_move_layer(float offset) {
		_curr_entity_position.z += offset;
		_update_entity_transform();
	}
	void Selection::_rotate(Angle offset) {
		_curr_entity_rotation += offset;
		_update_entity_transform();
	}
	void Selection::_rotate(glm::vec2 pivot, Angle offset) {
		auto pos = _curr_entity_position;
		auto obj_pivot = pos.xy() - _world_cam.screen_to_world(pivot, pos).xy();

		_curr_entity_position += vec3(rotate(obj_pivot, offset)-obj_pivot, 0.f);
		_curr_entity_rotation += offset;
		_update_entity_transform();
	}
	void Selection::_scale(float factor) {
		_curr_entity_scale *= factor;
		_update_entity_transform();
	}
	void Selection::_scale(glm::vec2 pivot, float factor) {
		auto pos = _curr_entity_position;
		auto obj_pivot = pos.xy() - _world_cam.screen_to_world(pivot, pos).xy();
		_curr_entity_position += vec3((obj_pivot*factor)-obj_pivot, 0.f);

		_curr_entity_scale *= factor;
		_update_entity_transform();
	}

	void Selection::_update_entity_transform() {
		if(_selected_entity) {
			auto& transform = _selected_entity->get<physics::Transform_comp>().get_or_throw();

			// TODO: snap to grid
			if(_snap_to_grid) {
				auto rp = glm::round(_curr_entity_position*2.f)/2.f;
				rp.z = glm::round(_curr_entity_position.z*10.f)/10.f;
				transform.position(rp*1_m);
				transform.rotation(glm::round(_curr_entity_rotation/15_deg) * 15_deg);
				transform.scale(glm::ceil(_curr_entity_scale*4.f)/4.f);

			} else {
				transform.position(_curr_entity_position*1_m);
				transform.rotation(_curr_entity_rotation);
				transform.scale(_curr_entity_scale);
			}
		}
	}

}
}
}
