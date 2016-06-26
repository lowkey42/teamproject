#pragma once

#include <core/utils/md5.hpp>
#include <core/utils/rest.hpp>

#include <string>
#include <vector>

namespace lux{

	class Highscore	{

		friend class Highscore_list;

		public:
			Highscore();

		private:
			std::string _name;
			int32_t _score;

	};

	class Highscore_list {

		public:
			Highscore_list(Highscore_list& other) {};
			Highscore_list(std::string lev_name);

		private:
			std::string _level;
			std::vector<Highscore> _scores;
			util::rest::Http_body _http_body;
			bool _loaded = false;

	};

}
