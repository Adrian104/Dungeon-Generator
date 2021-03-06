#include "dgen.hpp"

Dungeon::Dungeon(DGManager &pMgr, GenInfo &pGInfo, DrawInfo &pDInfo) : mgr(pMgr), gInfo(pGInfo), dInfo(pDInfo) {}
Dungeon::~Dungeon() {}

void Dungeon::GeneratePaths()
{
	Cell *const aCell = tree.Left();
	Cell *const bCell = tree.Right();

	if (aCell == nullptr || bCell == nullptr) return;

	SDL_Point &aPoint = aCell -> point;
	SDL_Point &bPoint = bCell -> point;
}

void Dungeon::Divide(int left)
{
	if (left <= 0)
	{
		end:
		Cell &crrCell = tree.Get();
		SDL_Rect room = crrCell.space;

		const int maxmin = gInfo.maxRoomSize - gInfo.minRoomSize;
		const int dX = int(room.w * ((gInfo.minRoomSize + rand() % maxmin) / 100.0f));
		const int dY = int(room.h * ((gInfo.minRoomSize + rand() % maxmin) / 100.0f));

		room.w -= dX;
		room.h -= dY;

		room.x += int(dX * (rand() % 100) / 100.0f);
		room.y += int(dY * (rand() % 100) / 100.0f);

		crrCell.point.x = room.x + int(room.w * (rand() % 100) / 100.0f);
		crrCell.point.y = room.y + int(room.h * (rand() % 100) / 100.0f);

		rooms.insert(std::make_pair(&crrCell, room));
		return;
	}
	else if (int delta = gInfo.maxDepth - gInfo.minDepth; left <= delta)
	{
		if (rand() % (delta + 1) >= left) goto end;
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

	auto DrawPoints = [](Dungeon *dg) -> void
	{
		SDL_Point &point = dg -> tree.Get().point;
		SDL_Rect rect = { point.x - 2, point.y - 2, 4, 4 };

		dg -> mgr.vPort.RectToScreen(rect, rect);
		SDL_RenderFillRect(dg -> mgr.renderer, &rect);
	};

	ExeHelper<Cell> helper(false, 0, [](const ExeInfo<Cell> &info) -> bool { return info.node.IsLast(); });

	if (dInfo.spaceVisibility)
	{
		SDL_SetRenderDrawColor(mgr.renderer, 0xFF, 0, 0, 0xFF);
		tree.Execute(helper, &DrawSpace, this);
	}

	if (dInfo.roomsVisibility)
	{
		SDL_SetRenderDrawColor(mgr.renderer, 0, 0xAA, 0xAA, 0xFF);
		for (auto &[key, room] : rooms)
		{
			SDL_Rect rect = room;
			mgr.vPort.RectToScreen(rect, rect);
			SDL_RenderDrawRect(mgr.renderer, &rect);
		}
	}

	if (dInfo.pointsVisibility)
	{
		SDL_SetRenderDrawColor(mgr.renderer, 0, 0xFF, 0, 0xFF);
		tree.Execute(helper, &DrawPoints, this);
	}
}

void Dungeon::Generate()
{
	rooms.clear();

	tree.ToRoot();
	tree.DeleteNode();

	tree.Get().space.w = gInfo.xSize;
	tree.Get().space.h = gInfo.ySize;

	Divide(gInfo.maxDepth);

	ExeHelper<Cell> helper(true, 0, [](const ExeInfo<Cell> &info) -> bool { return true; });
	tree.ExecuteObj(helper, &Dungeon::GeneratePaths, this);
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
	gInfo.maxDepth = 5;
	gInfo.minDepth = 4;
	gInfo.maxRoomSize = 75;
	gInfo.minRoomSize = 25;
	gInfo.spaceSizeRandomness = 35;

	dInfo.spaceVisibility = true;
	dInfo.roomsVisibility = true;
	dInfo.pointsVisibility = true;
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
		needRedraw = true;
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

			case SDLK_F3:
				dInfo.roomsVisibility = !dInfo.roomsVisibility;
				break;

			case SDLK_F4:
				dInfo.pointsVisibility = !dInfo.pointsVisibility;
				break;

			case SDLK_g:
				dg.Generate();
				break;

			default:
				vPort.Update(sdlEvent);
			}
		}
		else vPort.Update(sdlEvent);
	}
}