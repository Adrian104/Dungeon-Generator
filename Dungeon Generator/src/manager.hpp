#pragma once
#include "macros.hpp"
#include "dgen.hpp"

struct DrawInfo
{
	bool spaceVisibility;
	bool roomsVisibility;
	int nodesVisibilityMode;
	int pathsVisibilityMode;
};

struct DGManager
{
	int xSize;
	int ySize;

	bool quit;
	bool refTexture;

	Dungeon dg;
	DrawInfo dInfo;
	Viewport vPort;
	GenInput gInput;

	SDL_Window *window;
	SDL_Texture *texture;
	SDL_Renderer *renderer;

	DGManager();
	~DGManager();

	void Run();
	void Draw();
	void Update();
	void Refresh();
	void Generate();

	void Render();
	void ApplyFactor(const float factor);
};