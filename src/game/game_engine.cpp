#include "game_engine.hpp"

#include "highscore_manager.hpp"


namespace lux {

	Game_engine::Game_engine(const std::string& title, int argc, char** argv, char** env)
	    : Engine(title, argc, argv, env),
	      _highscore_manager(std::make_unique<Highscore_manager>(assets())) {
	}
	Game_engine::~Game_engine() {
		screens().clear(); // destroy all screens before the engine
	}

	void Game_engine::_on_frame(Time dt) {
		_highscore_manager->update(dt);
	}

}
