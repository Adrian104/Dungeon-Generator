#pragma once
#include "global.hpp"

inline const int gFontCount = sizeof(gFonts) / sizeof(*gFonts);

struct AppManager
{
	struct TextTx
	{
		int width, height;
		SDL_Texture *texture;
	};

	bool errorFlag;
	int windowWidth;
	int windowHeight;

	TextTx text;
	SDL_Window *window;
	SDL_Renderer *renderer;
	TTF_Font *fonts[gFontCount];

	AppManager(const std::string &pTitle);
	~AppManager();

	void Error(const std::string &msg);
	void RenderText(const std::string &str, int fontId, int style, const SDL_Color &color = { 0xFF, 0xFF, 0xFF, 0xFF });
};