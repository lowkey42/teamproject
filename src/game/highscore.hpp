#pragma once

#include <string>
#include <vector>

#include <core/asset/asset_manager.hpp>

namespace lux{

	/*struct Highscore_loading_failed : public asset::Loading_failed {
		explicit Highscore_loading_failed(const std::string& msg)noexcept : Loading_failed(msg){}
	};*/

	class Highscore	{

		public:
			Highscore() = default;
			//explicit Highscore(asset::istream stream) throw(Highscore_loading_failed);
			Highscore(std::string n, int32_t s) : _name(n), _score(s){}

			std::string _name;
			int32_t _score;

	};

	class Highscore_list {

		public:
			Highscore_list() = default;
			Highscore_list(std::string level);

			std::string _level;
			std::vector<Highscore> _scores;

		private:
			bool fetching;

	};

/*
namespace asset {
	template<>
	struct Loader<Highscore> {
		using RT = std::shared_ptr<Highscore>;

		static RT load(istream in) throw(Loading_failed){
			return std::make_unique<Highscore>(std::move(in));
		}

		static void store(ostream out, const Highscore& asset) throw(Loading_failed) {
			FAIL("NOT IMPLEMENTED, YET!");
		}
	};
}*/
}
