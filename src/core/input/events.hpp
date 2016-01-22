/**************************************************************************\
 * event-types caused by user-input                                       *
 *                                               ___                      *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2014 Florian Oetke                                       *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include "types.hpp"

#include <memory>
#include <glm/vec2.hpp>


namespace mo {
namespace input {

	struct Char_input {std::string character;};

	struct Source_added {
		Input_source src;
	};
	struct Source_removed {
		Input_source src;
	};


	// mapped inputs
	struct Once_action {
		Action_id id;
		Input_source src;
	};
	inline bool operator==(const Once_action& lhs, const Once_action& rhs) {
		return lhs.id==rhs.id && lhs.src==rhs.src;
	}

	struct Range_action {
		Action_id id;
		Input_source src;
		glm::vec2 value;
	};

	struct Force_feedback {
		Input_source src;
		float force; //< 0-1
	};
}
}
