/** The commands used by the editor (for undo/redo) **************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "editor_comp.hpp"

#include "selection.hpp"


namespace lux {
namespace editor {

	struct Delete_cmd : util::Command {
		public:
			Delete_cmd(Selection& selection);

			void execute()override;
			void undo()override;
			auto name()const -> const std::string& override {
				return _name;
			}

		private:
			const std::string _name;

			Selection& _selection;
			ecs::Entity_facet _entity;
			std::string _saved_state;
	};

	struct Paste_cmd : util::Command {
		public:
			Paste_cmd(ecs::Entity_manager& ecs,
			          Selection& selection, std::string data, glm::vec3 pos);

			void execute()override;
			void undo()override;
			auto name()const -> const std::string& override {
				return _name;
			}

		private:
			const std::string _name;

			ecs::Entity_manager& _ecs;
			Selection& _selection;
			ecs::Entity_facet _entity;
			std::string _data;
			glm::vec3 _pos;
	};

	struct Flip_cmd : util::Command {
		public:
			Flip_cmd(ecs::Entity_facet entity, bool vert);

			void execute()override;
			void undo()override;
			auto name()const -> const std::string& override {
				return _name;
			}

		private:
			const std::string _name;

			ecs::Entity_facet _entity;
			bool _vert;
	};


	struct Create_cmd : util::Command {
		public:
			Create_cmd(ecs::Entity_manager& ecs, Selection& selection, std::string blueprint, glm::vec3 pos);

			void execute()override;
			void undo()override;
			auto name()const -> const std::string& override {
				return _name;
			}

		private:
			const std::string _name;

			ecs::Entity_manager& _ecs;
			Selection& _selection;
			ecs::Entity_facet _entity;
			ecs::Entity_facet _entity_prev_selected;
			std::string _blueprint;
			std::string _data;
			glm::vec3 _pos;
	};


	struct Selection_change_cmd : util::Command {
		public:
			Selection_change_cmd(Selection& mgr, ecs::Entity_facet e);

			void execute()override;
			void undo()override;
			auto name()const -> const std::string& override {
				return _name;
			}

		private:
			const std::string _name;
			Selection& _selection_mgr;

			ecs::Entity_facet _selection;
			ecs::Entity_facet _prev_selection;
	};

	struct Transform_cmd : util::Command {
		public:
			Transform_cmd(Selection& selection_mgr, ecs::Entity_facet e, bool copy,
			              Position new_pos,  Angle new_rot,  float new_scale,
			              Position prev_pos, Angle prev_rot, float prev_scale);

			void execute()override;
			void undo()override;
			auto name()const -> const std::string& override {
				return _name;
			}

		private:
			const std::string _name;

			Selection& _selection_mgr;
			ecs::Entity_facet _entity;
			bool              _copied_entity;
			std::string       _saved_state;

			Position  _new_position;
			Angle     _new_rotation;
			float     _new_scale;

			Position  _prev_position;
			Angle     _prev_rotation;
			float     _prev_scale;
	};

	struct Point_deleted_cmd : util::Command {
		public:
			/// positions in model-space
			Point_deleted_cmd(ecs::Entity_facet e, int index, glm::vec2 prev_pos);

			void execute()override;
			void undo()override;
			auto name()const -> const std::string& override {
				return _name;
			}

		private:
			const std::string _name;

			ecs::Entity_facet _entity;

			bool      _first_exec = true;
			int       _index;
			glm::vec2 _prev_pos;
	};

	struct Point_moved_cmd : util::Command {
		public:
			/// positions in model-space
			Point_moved_cmd(ecs::Entity_facet e, int index, bool new_point, glm::vec2 new_pos, glm::vec2 prev_pos);
			void execute()override;
			void undo()override;
			auto name()const -> const std::string& override {
				return _name;
			}

		private:
			const std::string _name;

			ecs::Entity_facet _entity;

			bool      _first_exec = true;
			int       _index;
			bool      _new_point;
			glm::vec2 _new_pos;
			glm::vec2 _prev_pos;
	};

}
}
