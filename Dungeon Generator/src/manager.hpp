#pragma once
#include "dgen.hpp"

struct DrawInfo
{
	bool spaceVisibility;
	bool roomsVisibility;
};

struct DGManager
{
	bool quit;
	bool needRedraw;

	Dungeon dg;
	GenInfo gInfo;
	DrawInfo dInfo;
	Viewport vPort;

	SDL_Window *window;
	SDL_Renderer *renderer;

	DGManager();
	~DGManager();

	void Run();
	void Draw();
	void Update();
};