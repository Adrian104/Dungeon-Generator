#include "manager.hpp"

DGManager::DGManager() : quit(false), needRedraw(true), xSize(0), ySize(0)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	#ifdef FULL_SCREEN
		window = SDL_CreateWindow("Dungeon Generator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w, dm.h, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
		xSize = dm.w;
		ySize = dm.h;
	#else
		window = SDL_CreateWindow("Dungeon Generator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w - 30, dm.h - 100, SDL_WINDOW_SHOWN);
		xSize = dm.w - 30;
		ySize = dm.h - 100;
	#endif

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	gInfo.xSize = xSize;
	gInfo.ySize = ySize;

	gInfo.minDepth = 9;
	gInfo.maxDepth = 10;
	gInfo.maxRoomSize = 75;
	gInfo.minRoomSize = 25;
	gInfo.doubleRoomProb = 25;
	gInfo.spaceSizeRandomness = 35;

	dInfo.spaceVisibility = true;
	dInfo.roomsVisibility = true;
	dInfo.nodesVisibility = true;
	dInfo.pathsVisibilityMode = 1;
}

DGManager::~DGManager()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void DGManager::Run()
{
	dg.Generate(&gInfo);
	while (!quit)
	{
		if (needRedraw) Draw();
		Update();
		SDL_Delay(5);
	}
}

void DGManager::Draw()
{
	ExeHelper<Cell> helper(false, 0, [](const ExeInfo<Cell> &info) -> bool { return info.node.IsLast(); });

	auto DrawSpace = [](DGManager *mgr) -> void
	{
		SDL_FRect rect = ToFRect(mgr -> dg.tree.Get().space);
		mgr -> vPort.RectToScreen(rect, rect);
		SDL_RenderDrawRectF(mgr -> renderer, &rect);
	};

	auto DrawRooms = [](DGManager *mgr) -> void
	{
		for (Room *room = mgr -> dg.tree.Get().roomList; room != nullptr; room = room -> nextRoom)
		{
			SDL_FRect rect = ToFRect(room -> room);
			mgr -> vPort.RectToScreen(rect, rect);
			SDL_RenderDrawRectF(mgr -> renderer, &rect);
		}
	};

	auto DrawNodes = [](DGManager *mgr) -> void
	{
		auto end = mgr -> dg.pNodes.end();
		for (auto iter = mgr -> dg.pNodes.begin(); iter != end; iter++)
		{
			SDL_FPoint point = ToFPoint(iter -> pos);
			SDL_FRect rect = { point.x, point.y, 1, 1 };

			mgr -> vPort.RectToScreen(rect, rect);
			SDL_RenderFillRectF(mgr -> renderer, &rect);
		}
	};

	auto DrawLinks = [](DGManager *mgr) -> void
	{
		auto end = mgr -> dg.pNodes.end();
		for (auto iter = mgr -> dg.pNodes.begin(); iter != end; iter++)
		{
			#ifdef RANDOM_COLORS
			int r, g, b;

			do
			{
				r = 255 * (rand() & 0x1);
				g = 255 * (rand() & 0x1);
				b = 255 * (rand() & 0x1);
			} while (r == 0 && g == 0 && b == 0);

			SDL_SetRenderDrawColor(mgr -> renderer, r, g, b, 0xFF);
			#endif

			for (int i = 0; i < 2; i++)
			{
				if (iter -> links[i] == nullptr) continue;

				SDL_FPoint p1 = ToFPoint(iter -> pos);
				SDL_FPoint p2 = ToFPoint(iter -> links[i] -> pos);

				p1.x += 0.5f; p1.y += 0.5f;
				p2.x += 0.5f; p2.y += 0.5f;

				mgr -> vPort.ToScreen(p1.x, p1.y, p1.x, p1.y);
				mgr -> vPort.ToScreen(p2.x, p2.y, p2.x, p2.y);

				SDL_RenderDrawLineF(mgr -> renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}
	};

	auto DrawPaths = [](DGManager *mgr) -> void
	{
		auto end = mgr -> dg.pNodes.end();
		for (auto iter = mgr -> dg.pNodes.begin(); iter != end; iter++)
		{
			for (int i = 0; i < 4; i++)
			{
				if (!(iter -> path & (1 << i))) continue;

				SDL_FPoint p1 = ToFPoint(iter -> pos);
				SDL_FPoint p2 = ToFPoint(iter -> links[i] -> pos);

				p1.x += 0.5f; p1.y += 0.5f;
				p2.x += 0.5f; p2.y += 0.5f;

				mgr -> vPort.ToScreen(p1.x, p1.y, p1.x, p1.y);
				mgr -> vPort.ToScreen(p2.x, p2.y, p2.x, p2.y);

				SDL_RenderDrawLineF(mgr -> renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}
	};

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	if (dInfo.spaceVisibility)
	{
		SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
		dg.tree.Execute(helper, &DrawSpace, this);
	}

	if (dInfo.roomsVisibility)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0xAA, 0xAA, 0xFF);
		dg.tree.Execute(helper, &DrawRooms, this);
	}

	if (dInfo.pathsVisibilityMode == 1)
	{
		SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0x55, 0xFF);
		DrawLinks(this);
	}

	if (dInfo.pathsVisibilityMode == 2)
	{
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		DrawPaths(this);
	}

	if (dInfo.nodesVisibility)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);
		DrawNodes(this);
	}

	#ifdef SHOW_GRID
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x16);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	for (float x = 0; x <= gInfo.xSize; x++)
	{
		SDL_FPoint a = { x, 0 };
		SDL_FPoint b = { x, float(gInfo.ySize) };

		vPort.ToScreen(a.x, a.y, a.x, a.y);
		vPort.ToScreen(b.x, b.y, b.x, b.y);

		SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
	}

	for (float y = 0; y <= gInfo.ySize; y++)
	{
		SDL_FPoint a = { 0, y };
		SDL_FPoint b = { float(gInfo.xSize), y };

		vPort.ToScreen(a.x, a.y, a.x, a.y);
		vPort.ToScreen(b.x, b.y, b.x, b.y);

		SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
	}
	#endif

	SDL_RenderPresent(renderer);
	needRedraw = false;
}

void DGManager::Update()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		needRedraw = true;
		if (sdlEvent.type == SDL_QUIT) quit = true;
		else if (sdlEvent.type == SDL_KEYDOWN)
		{
			switch (sdlEvent.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				quit = true;
				break;

			case SDLK_F1:
				dInfo.spaceVisibility = !dInfo.spaceVisibility;
				break;

			case SDLK_F2:
				dInfo.roomsVisibility = !dInfo.roomsVisibility;
				break;

			case SDLK_F3:
				dInfo.nodesVisibility = !dInfo.nodesVisibility;
				break;

			case SDLK_F4:
				dInfo.pathsVisibilityMode++;
				if (dInfo.pathsVisibilityMode > 2) dInfo.pathsVisibilityMode = 0;
				break;

			case SDLK_g:
				dg.Generate(&gInfo);
				Draw();
				break;

			default:
				vPort.Update(sdlEvent);
			}
		}
		else vPort.Update(sdlEvent);
	}
}

void DGManager::ApplyFactor(const float factor)
{
	gInfo.xSize = int(xSize * factor);
	gInfo.ySize = int(ySize * factor);

	vPort.SetDefaultScale(1 / factor);
	vPort.Reset();
}