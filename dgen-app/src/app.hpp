#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "vport.hpp"
#include "dgen.hpp"
#include "appmgr.hpp"
#include "overlay.hpp"

struct Application : public AppManager
{
	struct DrawInfo
	{
		bool roomsVisibility;
		bool pathsVisibility;
		bool entrancesVisibility;
	};

	enum class GenMode { NEXT_SEED, RAND_SEED, REFRESH, DEBUG };

	bool plus;
	bool fullscreen;
	float factor, lastFactor;

	Generator gen;
	GenInput gInput;
	GenOutput gOutput;

	DrawInfo dInfo;
	Viewport vPort;
	Overlay *overlay;
	SDL_Texture *texture;

	std::random_device rd;
	Random::seed_type seed;

	Application();
	~Application();

	void Run();
	void Draw();
	bool Update();

	void Render();
	void RenderDebug();
	void RenderCommon();

	void InitWindow();
	void ApplyFactor();
	void LoadDefaults();
	void Generate(GenMode mode);
};