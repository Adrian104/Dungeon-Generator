#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "global.hpp"

struct AppManager
{
	struct TextTx
	{
		int width, height;
		SDL_Texture *texture;
	};

	int windowWidth;
	int windowHeight;

	TextTx text;
	SDL_Window *window;
	SDL_Renderer *renderer;
	TTF_Font *fonts[gFontCount];

	AppManager();
	~AppManager();

	void RenderText(const std::string &str, int fontId, int style, const SDL_Color &color = { 0xFF, 0xFF, 0xFF, 0xFF });
};