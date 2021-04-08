#pragma once
#include "macros.hpp"
#include "dgen.hpp"

struct DrawInfo
{
	bool spaceVisibility;
	bool roomsVisibility;
	bool nodesVisibility;
	int pathsVisibilityMode;
};

struct DGManager
{
	bool quit;
	bool needRedraw;

	int xSize;
	int ySize;

	Dungeon dg;
	DrawInfo dInfo;
	Viewport vPort;
	GenInput gInput;

	SDL_Window *window;
	SDL_Renderer *renderer;

	DGManager();
	~DGManager();

	void Run();
	void Draw();
	void Update();

	void ApplyFactor(const float factor);
};