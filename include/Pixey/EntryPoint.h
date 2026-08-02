#pragma once

// Include EXACTLY ONCE, in the .cpp that uses PIXEY_RUN.

#include "Pixey/Pixey.h"

#include <SDL3/SDL_main.h>

#define PIXEY_RUN(ClassName)                              \
	int main(int argc, char* argv[])                      \
	{                                                     \
		ClassName game;                                   \
		return ::Pixey::RunApp(game, argc, argv);         \
	}
