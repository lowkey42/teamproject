/** application's entry point ************************************************
 *                                                                           *
 * Copyright (c) 2014 Florian Oetke                                          *
 *  This file is distributed under the MIT License                           *
 *  See LICENSE file for details.                                            *
\*****************************************************************************/

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "core/utils/log.hpp"
#include "core/utils/stacktrace.hpp"

#include "core/engine.hpp"

#include "game/editor_screen.hpp"

#include "info.hpp"

#include <iostream>
#include <exception>
#include <SDL2/SDL.h>

using namespace lux; // import game namespace

std::unique_ptr<Engine> engine;

void init(int argc, char** argv, char** env);
void onFrame();
void shutdown();


#ifdef main
int main(int argc, char** argv) {
	char* noEnv = nullptr;
	char** env = &noEnv;
#else
int main(int argc, char** argv, char** env) {
#endif

	#ifdef EMSCRIPTEN
		init(argc, argv, env);

		emscripten_set_main_loop(onFrame, 0, 1);

		shutdown();
		emscripten_exit_with_live_runtime();
	#else
		init(argc, argv, env);

		while(engine->running())
			onFrame();

		shutdown();
	#endif

    return 0;
}

void init(int argc, char** argv, char** env) {
	INFO("Game started from: "<<argv[0]<<"\n"
	     <<"Working dir: "<<asset::pwd()<<"\n"
	     <<"Version: "<<version_info::name<<"\n"
	     <<"Version-Hash: "<<version_info::hash<<"\n"
	     <<"Version-Date: "<<version_info::date<<"\n"
	     <<"Version-Subject: "<<version_info::subject<<"\n");
	// TODO: print system information

	try {
		util::init_stacktrace(argv[0]);
		engine.reset(new Engine("Teamproject", argc, argv, env));
		engine->screens().enter<Editor_screen>();

	} catch (const util::Error& ex) {
		CRASH_REPORT("Exception in init: "<<ex.what());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Sorry :-(", "Error in init", nullptr);
		shutdown();
		exit(1);
	}
}

void onFrame() {
	try {
		engine->on_frame();

	} catch (const util::Error& ex) {
		CRASH_REPORT("Exception in onFrame: "<<ex.what());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Sorry :-(", "Error in onFrame", nullptr);
		shutdown();
		exit(2);
	}
}

void shutdown() {
	try {
		engine.reset();

	} catch (const util::Error& ex) {
		CRASH_REPORT("Exception in shutdown: "<<ex.what());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Sorry :-(", "Error in shutdown", nullptr);
		exit(3);
	}
}
