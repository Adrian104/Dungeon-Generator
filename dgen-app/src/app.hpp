#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "vport.hpp"
#include "dgen.hpp"
#include "appmgr.hpp"
#include "overlay.hpp"

struct Application : public AppManager
{
	enum class GenMode { NEXT_SEED, RAND_SEED, REFRESH };

	bool plus;
	bool debug;
	float factor;
	bool fullscreen;

	bool visRooms;
	bool visPaths;
	bool visEntrances;

	Generator gen;
	GenInput gInput;
	GenOutput gOutput;

	Viewport vPort;
	Overlay *overlay;
	SDL_Texture *texture;

	std::random_device rd;
	Random::seed_type seed;

	Application();
	~Application();

	void Run();
	void Draw();
	void Render();
	bool Update();

	void InitWindow();
	void LoadDefaults();
	void Generate(GenMode mode);
};