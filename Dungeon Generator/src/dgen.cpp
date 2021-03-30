#include "dgen.hpp"

#ifndef _DEBUG
#define FULL_SCREEN
#endif

PNode PNode::null = PNode({ 0, 0 });

Dungeon::Dungeon() : cache{}, gInfo(nullptr), pXNodes(nullptr), pYNodes(nullptr) {}
Dungeon::~Dungeon() { Clear(); }

void Dungeon::Prepare()
{
	const size_t toReserve = 1 << (size_t(gInfo -> maxDepth) + 2);

	pXNodes = new std::unordered_multimap<float, PNode*>(toReserve);
	pYNodes = new std::unordered_multimap<float, PNode*>(toReserve);

	pXNodes -> max_load_factor(8);
	pYNodes -> max_load_factor(8);

	tree.Get().space.w = float(gInfo -> xSize) - 3;
	tree.Get().space.h = float(gInfo -> ySize) - 3;

	cache.deltaDepth = gInfo -> maxDepth - gInfo -> minDepth;
	cache.deltaRoomSize = gInfo -> maxRoomSize - gInfo -> minRoomSize;
}

void Dungeon::MakeRoom()
{
	Cell &crrCell = tree.Get();
	SDL_FRect room = crrCell.space;
	SDL_FRect room2;

	const bool doubleRoom = (rand() % 100) < gInfo -> doubleRoomProb;

	float dX = room.w * ((gInfo -> minRoomSize + rand() % cache.deltaRoomSize) / 100.0f);
	float dY = room.h * ((gInfo -> minRoomSize + rand() % cache.deltaRoomSize) / 100.0f);

	room.w -= dX;
	room.h -= dY;

	if (doubleRoom)
	{
		room2 = room;

		if (dX > dY)
		{
			const float extra = dX * ((rand() % 100) / 100.0f);

			room2.h /= 2.0f;
			room2.w += extra;
			dX -= extra;

			if (rand() & 1) room2.y = room.y + room.h - room2.h;
		}
		else
		{
			const float extra = dY * ((rand() % 100) / 100.0f);

			room2.w /= 2.0f;
			room2.h += extra;
			dY -= extra;

			if (rand() & 1) room2.x = room.x + room.w - room2.w;
		}
	}

	const float xOffset = dX * (rand() % 100) / 100.0f;
	const float yOffset = dY * (rand() % 100) / 100.0f;

	room.x += xOffset;
	room.y += yOffset;

	crrCell.roomList = new Room;
	crrCell.roomList -> room = room;

	if (doubleRoom)
	{
		room2.x += xOffset;
		room2.y += yOffset;

		crrCell.roomList -> nextRoom = new Room;
		crrCell.roomList -> nextRoom -> room = room2;
	}
}

void Dungeon::Divide(int left)
{
	if (left <= 0)
	{
		nomore:
		SDL_FRect &space = tree.Get().space;

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

	float SDL_FRect::*xy;
	float SDL_FRect::*wh;

	SDL_FRect *crrSpace = &tree.Get().space;

	const bool vert = crrSpace -> w > crrSpace -> h;
	if (vert) { xy = &SDL_FRect::x; wh = &SDL_FRect::w; }
	else { xy = &SDL_FRect::y; wh = &SDL_FRect::h; }

	float randSize;
	const float totalSize = crrSpace ->* wh;

	if (gInfo -> spaceSizeRandomness > 0) randSize = totalSize / 2.0f - totalSize * ((gInfo -> spaceSizeRandomness >> 1) / 100.0f) + totalSize * ((rand() % gInfo -> spaceSizeRandomness) / 100.0f);
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