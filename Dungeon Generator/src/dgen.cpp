#include "dgen.hpp"

#ifndef _DEBUG
#define FULL_SCREEN
#endif

PNode PNode::null = PNode({ 0, 0 });

Dungeon::Dungeon() : cache{}, gInfo(nullptr), pXNodes(nullptr), pYNodes(nullptr) {}
Dungeon::~Dungeon() { Clear(); }

void Dungeon::Prepare()
{
	const size_t toReserve = size_t(1) << (size_t(gInfo -> maxDepth) + 2);

	pXNodes = new std::unordered_multimap<int, PNode*>(toReserve);
	pYNodes = new std::unordered_multimap<int, PNode*>(toReserve);

	pXNodes -> max_load_factor(8);
	pYNodes -> max_load_factor(8);

	tree.Get().space.w = gInfo -> xSize - 3;
	tree.Get().space.h = gInfo -> ySize - 3;

	cache.deltaDepth = gInfo -> maxDepth - gInfo -> minDepth;
	cache.deltaRoomSize = gInfo -> maxRoomSize - gInfo -> minRoomSize;
}

void Dungeon::MakeRoom()
{
	Cell &crrCell = tree.Get();
	SDL_Rect room = crrCell.space;
	SDL_Rect room2;

	const bool doubleRoom = (rand() % 100) < gInfo -> doubleRoomProb;

	int dX = int(room.w * ((gInfo -> minRoomSize + rand() % cache.deltaRoomSize) / 100.0f));
	int dY = int(room.h * ((gInfo -> minRoomSize + rand() % cache.deltaRoomSize) / 100.0f));

	room.w -= dX;
	room.h -= dY;

	if (doubleRoom)
	{
		room2 = room;

		if (dX > dY)
		{
			const int extra = int(dX * ((rand() % 100) / 100.0f));

			room2.h >>= 1;
			room2.w += extra;
			dX -= extra;

			if (rand() & 1) room2.y = room.y + room.h - room2.h;
		}
		else
		{
			const int extra = int(dY * ((rand() % 100) / 100.0f));

			room2.w >>= 1;
			room2.h += extra;
			dY -= extra;

			if (rand() & 1) room2.x = room.x + room.w - room2.w;
		}
	}

	const int xOffset = int(dX * (rand() % 100) / 100.0f);
	const int yOffset = int(dY * (rand() % 100) / 100.0f);

	room.x += xOffset;
	room.y += yOffset;

	crrCell.roomList = new Room(room);

	if (doubleRoom)
	{
		room2.x += xOffset;
		room2.y += yOffset;

		crrCell.roomList -> nextRoom = new Room(room2);
	}
}

void Dungeon::Divide(int left)
{
	if (left <= 0)
	{
		nomore:
		SDL_Rect &space = tree.Get().space;

		space.x += 4; space.y += 4;
		space.w -= 5; space.h -= 5;

		MakeRoom();
		return;
	}
	else if (left <= cache.deltaDepth)
	{
		if (rand() % (cache.deltaDepth + 1) >= left) goto nomore;
	}

	left--;

	int SDL_Rect::*xy;
	int SDL_Rect::*wh;

	SDL_Rect *crrSpace = &tree.Get().space;
	const bool vert = crrSpace -> w > crrSpace -> h;

	if (vert) { xy = &SDL_Rect::x; wh = &SDL_Rect::w; }
	else { xy = &SDL_Rect::y; wh = &SDL_Rect::h; }

	const int totalSize = crrSpace ->* wh;
	int randSize = totalSize >> 1;

	if (gInfo -> spaceSizeRandomness > 0) randSize += int(totalSize * ((rand() % gInfo -> spaceSizeRandomness) / 100.0f) - totalSize * ((gInfo -> spaceSizeRandomness >> 1) / 100.0f));

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

void Dungeon::Clear()
{
	delete pYNodes;
	pYNodes = nullptr;

	delete pXNodes;
	pXNodes = nullptr;

	pNodes.clear();

	tree.ToRoot();
	tree.DeleteNode();
}

void Dungeon::Generate(GenInfo *genInfo)
{
	Clear();
	gInfo = genInfo;

	Prepare();
	Divide(gInfo -> maxDepth);
}