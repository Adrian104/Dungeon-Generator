#include "dgen.hpp"

Dungeon::Dungeon(DGManager *const mgrPtr) : xSize(0), ySize(0), mgr(mgrPtr) {}
Dungeon::~Dungeon() {}

void Dungeon::Draw()
{

}

void Dungeon::Generate()
{

}

DGManager::DGManager() : quit(false), dg(this), vPort(0.1f)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	window = SDL_CreateWindow("Dungeon Generator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w - 30, dm.h - 30, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
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
		Draw();
		Update();
		SDL_Delay(50);
	}
}

void DGManager::Draw()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}

void DGManager::Update()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		if (sdlEvent.type == SDL_QUIT) quit = true;
		else vPort.Update(sdlEvent);
	}
}