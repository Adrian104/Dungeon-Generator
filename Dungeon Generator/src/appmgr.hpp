#pragma once
#include <SDL.h>
#include "macros.hpp"

struct AppManager
{
	int windowWidth;
	int windowHeight;

	SDL_Window *window;
	SDL_Renderer *renderer;

	AppManager();
	~AppManager();
};