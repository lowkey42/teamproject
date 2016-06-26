#include "highscore.hpp"

namespace lux {

	Highscore::Highscore() {
		_name = std::string("");
		_score = -1;
	}

	Highscore_list::Highscore_list(std::string lev_name){



		std::string host = "localhost";
		int port = 80;
		std::string path = "/databasePHP/database.php";

		_level = lev_name;
		_scores = std::vector<Highscore>();
		util::rest::Http_parameters params;
		params["game"] = "Into_the_light";
		params["level"] = lev_name;
		params["op"] = "ghigh";
		_http_body = util::rest::get_request(host, port, path, params);
	}

}
