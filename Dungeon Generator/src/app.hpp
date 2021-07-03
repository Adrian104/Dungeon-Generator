#pragma once
#include "global.hpp"
#include "vport.hpp"
#include "core/dgen.hpp"
#include "overlay.hpp"

struct DrawInfo
{
	bool roomsVisibility;
	bool pathsVisibility;
	bool entrancesVisibility;
};

struct Application : public AppManager
{
	enum class GenMode : byte { OLD_SEED, NEW_SEED, DEBUG_MODE };

	bool quit, plus;
	float factor, lastFactor;

	GenInput gInput;
	GenOutput *gOutput;

	Generator gen;
	DrawInfo dInfo;
	Viewport vPort;
	Overlay *overlay;
	SDL_Texture *texture;

	Application();
	~Application();

	void Run();
	void Draw();
	void Update();

	void Render();
	void RenderDebug();

	void ApplyFactor();
	void LoadDefaults();
	void Generate(GenMode mode);
};