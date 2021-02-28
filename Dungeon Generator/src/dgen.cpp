#include "dgen.hpp"

Dungeon::Dungeon(DGManager &pMgr, GenInfo &pGInfo, DrawInfo &pDInfo) : mgr(pMgr), gInfo(pGInfo), dInfo(pDInfo) {}
Dungeon::~Dungeon() {}

void Dungeon::Divide(int left)
{
	if (left <= 0)
	{
		// TODO: Make rooms.
		return;
	}

	left--;

	int SDL_Rect::*xy;
	int SDL_Rect::*wh;

	SDL_Rect *crrSpace = &tree.Get().space;

	if (crrSpace -> w > crrSpace -> h) { xy = &SDL_Rect::x; wh = &SDL_Rect::w; }
	else { xy = &SDL_Rect::y; wh = &SDL_Rect::h; }

	int randSize;
	const int totalSize = crrSpace ->* wh;

	if (gInfo.spaceSizeRandomness > 0) randSize = totalSize / 2 - int(totalSize * ((gInfo.spaceSizeRandomness >> 1) / 100.0f)) + int(totalSize * ((rand() % gInfo.spaceSizeRandomness) / 100.0f));
	else randSize = totalSize / 2;

	tree.AddNode(tree.Get());
	tree.GoLeft();
	crrSpace = &tree.Get().space;

	crrSpace ->* wh = randSize;

	Divide(left);
	tree.GoUp();
	tree.GoRight();
	crrSpace = &tree.Get().space;

	crrSpace ->* xy += randSize;
	crrSpace ->* wh -= randSize;

	Divide(left);
	tree.GoUp();
}

void Dungeon::Draw()
{
	auto DrawSpace = [](Dungeon *dg) -> void
	{
		SDL_Rect rect = dg -> tree.Get().space;
		dg -> mgr.vPort.RectToScreen(rect, rect);
		SDL_RenderDrawRect(dg -> mgr.renderer, &rect);
	};

	ExeHelper<Cell> helper(false, 0, [](const ExeInfo<Cell> &info) -> bool { return info.node.IsLast(); });

	if (dInfo.spaceVisibility)
	{
		SDL_SetRenderDrawColor(mgr.renderer, 0xFF, 0, 0, 0xFF);
		tree.Execute(helper, &DrawSpace, this);
	}
}

void Dungeon::Generate()
{
	tree.ToRoot();
	tree.DeleteNode();

	tree.Get().space.w = gInfo.xSize;
	tree.Get().space.h = gInfo.ySize;

	Divide(gInfo.maxDepth);
}

DGManager::DGManager() : quit(false), needRedraw(true), dg(*this, gInfo, dInfo), vPort(0.1f)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	//window = SDL_CreateWindow("Dungeon Generator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w, dm.h, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
	window = SDL_CreateWindow("Dungeon Generator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w - 30, dm.h - 60, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	gInfo.xSize = dm.w;
	gInfo.ySize = dm.h;
	gInfo.maxDepth = 4;
	gInfo.minDepth = 4; // TODO
	gInfo.spaceSizeRandomness = 35;

	dInfo.spaceVisibility = true;
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
		else if (sdlEvent.type == SDL_KEYDOWN)
		{
			switch (sdlEvent.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				quit = true;
				break;

			case SDLK_F2:
				dInfo.spaceVisibility = !dInfo.spaceVisibility;
				break;

			case SDLK_g:
				dg.Generate();
				break;

			default:
				vPort.Update(sdlEvent);
			}
		}
		else vPort.Update(sdlEvent);

		needRedraw = true;
	}
}