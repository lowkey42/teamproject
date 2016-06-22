#pragma once

#include <core/utils/md5.hpp>

#include <string>

namespace lux{

	class Highscore	{

		public:
			Highscore();

		private:
			std::string _name;
			std::string _level;
			int32_t _score;
			std::string _md5_hash;

	};
}
