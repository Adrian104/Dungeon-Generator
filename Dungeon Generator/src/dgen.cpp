#include "dgen.hpp"

Dungeon::Dungeon(DGManager *const mgrPtr) : xSize(0), ySize(0), mgr(mgrPtr) {}
Dungeon::~Dungeon() {}

void Dungeon::Draw()
{
	SDL_Rect rect;
	mgr -> vPort.RectToScreen(0, 0, xSize / 2.0f, ySize / 2.0f, rect);

	SDL_SetRenderDrawColor(mgr -> renderer, 0, 0xFF, 0, 0xFF);
	SDL_RenderFillRect(mgr -> renderer, &rect);
}

void Dungeon::Generate()
{

}

DGManager::DGManager() : quit(false), needRedraw(true), dg(this), vPort(0.1f)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	//window = SDL_CreateWindow("Dungeon Generator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w, dm.h, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
	window = SDL_CreateWindow("Dungeon Generator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w - 30, dm.h - 60, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	dg.SetSize(dm.w, dm.h);
}

DGManager::~DGManager()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void DGManager::Run()
{
	dg.Generate();
	while (!quit)
	{
		if (needRedraw) Draw();
		Update();
		SDL_Delay(5);
	}
}

void DGManager::Draw()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	dg.Draw();
	SDL_RenderPresent(renderer);

	needRedraw = false;
}

void DGManager::Update()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		if (sdlEvent.type == SDL_QUIT) quit = true;
		else vPort.Update(sdlEvent);

		needRedraw = true;
	}
}