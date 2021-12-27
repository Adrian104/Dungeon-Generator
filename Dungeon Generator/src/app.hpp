#pragma once
#include "global.hpp"
#include "vport.hpp"
#include "dgen.hpp"
#include "overlay.hpp"

struct Application : public AppManager
{
	struct DrawInfo
	{
		bool roomsVisibility;
		bool pathsVisibility;
		bool entrancesVisibility;
	};

	enum class GenMode : byte { OLD_SEED, NEW_SEED, DEBUG_MODE };

	bool plus;
	float factor, lastFactor;

	Generator gen;
	GenInput gInput;
	GenOutput gOutput;

	DrawInfo dInfo;
	Viewport vPort;
	Overlay *overlay;
	SDL_Texture *texture;

	Application();
	~Application();

	void Run();
	void Draw();
	bool Update();

	void Render();
	void RenderDebug();

	void ApplyFactor();
	void LoadDefaults();
	void Generate(GenMode mode);
};