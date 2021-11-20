#include "pch.hpp"
#include "appmgr.hpp"

AppManager::AppManager(const std::string &pTitle) : errorFlag(false), text(), fonts()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	#ifdef FULL_SCREEN
		window = SDL_CreateWindow(pTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w, dm.h, SDL_WINDOW_HIDDEN | SDL_WINDOW_FULLSCREEN);
		windowWidth = dm.w;
		windowHeight = dm.h;
	#else
		window = SDL_CreateWindow(pTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w - 30, dm.h - 100, SDL_WINDOW_HIDDEN);
		windowWidth = dm.w - 30;
		windowHeight = dm.h - 100;
	#endif

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	TTF_Font **crrFont = fonts;
	for (auto &[name, size] : gFonts)
	{
		TTF_Font *const font = TTF_OpenFont(name.c_str(), size);
		if (font == nullptr)
		{
			Error("File \"" + name + "\" not found");
			return;
		}

		*crrFont = font;
		crrFont++;
	}

	SDL_ShowWindow(window);
}

AppManager::~AppManager()
{
	if (text.texture != nullptr) SDL_DestroyTexture(text.texture);
	for (TTF_Font *&font : fonts) TTF_CloseFont(font);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	TTF_Quit();
	SDL_Quit();
}

void AppManager::Error(const std::string &msg)
{
	errorFlag = true;
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", msg.c_str(), window);
}

void AppManager::RenderText(const std::string &str, int fontId, int style, const SDL_Color &color)
{
	if (text.texture != nullptr) SDL_DestroyTexture(text.texture);

	TTF_Font *const font = fonts[fontId];
	TTF_SetFontStyle(font, style);

	SDL_Surface *surface = TTF_RenderText_Blended(font, str.c_str(), color);
	text.texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_FreeSurface(surface);
	SDL_QueryTexture(text.texture, nullptr, nullptr, &text.width, &text.height);
}