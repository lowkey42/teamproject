/** Simple API to communicate with an HTTP service ***************************
 *                                                                           *
 * Copyright (c) 2016 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#pragma once

#include "maybe.hpp"

#include <string>
#include <unordered_map>
#include <future>

namespace lux {
namespace util {
namespace rest {

	using Http_parameters = std::unordered_map<std::string, std::string>;
	using Http_body = std::future<std::string>;

	//< has to be called regularly to allow the requests to make progress
	extern void update();

	extern auto post_request(const std::string& host, int port, const std::string& path,
	                         const Http_parameters& get_param,
	                         const Http_parameters& post_param) -> Http_body;

	extern auto get_request(const std::string& host, int port, const std::string& path,
	                        const Http_parameters& get_param) -> Http_body;

	extern auto get_content(Http_body&) -> maybe<std::string>;
}
}
}
