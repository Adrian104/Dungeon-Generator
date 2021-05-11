#pragma once
#include "global.hpp"
#include "appmgr.hpp"
#include "dgen.hpp"
#include "overlay.hpp"

struct DrawInfo
{
	bool spaceVisibility;
	bool roomsVisibility;
	int nodesVisibilityMode;
	int pathsVisibilityMode;
};

struct Application : public AppManager
{
	bool quit, plus;
	float factor, lastFactor;

	Dungeon dg;
	DrawInfo dInfo;
	Viewport vPort;
	GenInput gInput;
	Overlay *overlay;
	SDL_Texture *texture;

	Application();
	~Application();

	void Run();
	void Draw();
	void Update();

	void Render();
	void ApplyFactor();
	void LoadDefaults();
	void Generate(const bool newSeed);
};