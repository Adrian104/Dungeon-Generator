#include "dgen.hpp"

Dungeon::Dungeon(DGManager &pMgr, GenInfo &pGInfo, DrawInfo &pDInfo) : mgr(pMgr), gInfo(pGInfo), dInfo(pDInfo) {}
Dungeon::~Dungeon() {}

void Dungeon::Divide(int left)
{
	if (left <= 0)
	{
		end:
		Cell &crrCell = tree.Get();
		SDL_FRect room = crrCell.space;

		const int maxmin = gInfo.maxRoomSize - gInfo.minRoomSize;
		const float dX = room.w * ((gInfo.minRoomSize + rand() % maxmin) / 100.0f);
		const float dY = room.h * ((gInfo.minRoomSize + rand() % maxmin) / 100.0f);

		room.w -= dX;
		room.h -= dY;

		room.x += dX * (rand() % 100) / 100.0f;
		room.y += dY * (rand() % 100) / 100.0f;

		crrCell.roomList = new Room;
		crrCell.roomList -> room = room;

		return;
	}
	else if (int delta = gInfo.maxDepth - gInfo.minDepth; left <= delta)
	{
		if (rand() % (delta + 1) >= left) goto end;
	}

	left--;

	float SDL_FRect::*xy;
	float SDL_FRect::*wh;

	SDL_FRect *crrSpace = &tree.Get().space;

	const bool vert = crrSpace -> w > crrSpace -> h;
	if (vert) { xy = &SDL_FRect::x; wh = &SDL_FRect::w; }
	else { xy = &SDL_FRect::y; wh = &SDL_FRect::h; }

	float randSize;
	const float totalSize = crrSpace ->* wh;

	if (gInfo.spaceSizeRandomness > 0) randSize = totalSize / 2.0f - totalSize * ((gInfo.spaceSizeRandomness >> 1) / 100.0f) + totalSize * ((rand() % gInfo.spaceSizeRandomness) / 100.0f);
	else randSize = totalSize / 2.0f;

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
		SDL_FRect rect = dg -> tree.Get().space;
		dg -> mgr.vPort.RectToScreen(rect, rect);
		SDL_RenderDrawRectF(dg -> mgr.renderer, &rect);
	};

	auto DrawRooms = [](Dungeon *dg) -> void
	{
		for (Room *room = dg -> tree.Get().roomList; room != nullptr; room = room -> nextRoom)
		{
			SDL_FRect rect = room -> room;
			dg -> mgr.vPort.RectToScreen(rect, rect);
			SDL_RenderDrawRectF(dg -> mgr.renderer, &rect);
		}
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
		tree.Execute(helper, &DrawRooms, this);
	}
}

void Dungeon::Generate()
{
	tree.ToRoot();
	tree.DeleteNode();

	tree.Get().space.w = float(gInfo.xSize);
	tree.Get().space.h = float(gInfo.ySize);

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
	gInfo.maxDepth = 9;
	gInfo.minDepth = 8;
	gInfo.maxRoomSize = 75;
	gInfo.minRoomSize = 25;
	gInfo.spaceSizeRandomness = 35;

	dInfo.spaceVisibility = true;
	dInfo.roomsVisibility = true;
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

			case SDLK_F1:
				dInfo.spaceVisibility = !dInfo.spaceVisibility;
				break;

			case SDLK_F2:
				dInfo.roomsVisibility = !dInfo.roomsVisibility;
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