#include "dgen.hpp"

#ifndef _DEBUG
#define FULL_SCREEN
#endif

Dungeon::Dungeon() : cache{}, gInfo(nullptr) {}
Dungeon::~Dungeon() { Clear(); }

void Dungeon::Prepare()
{
	gInfo -> xSize = int(RoundTo(float(gInfo -> xSize), gInfo -> tileSize));
	gInfo -> ySize = int(RoundTo(float(gInfo -> ySize), gInfo -> tileSize));

	tree.Get().space.w = float(gInfo -> xSize) - cache.tileSize3;
	tree.Get().space.h = float(gInfo -> ySize) - cache.tileSize3;

	cache.tileSize3 = gInfo -> tileSize * 3;
	cache.tileSize4 = gInfo -> tileSize * 4;
	cache.tileSize5 = gInfo -> tileSize * 5;
	cache.deltaDepth = gInfo -> maxDepth - gInfo -> minDepth;
	cache.deltaRoomSize = gInfo -> maxRoomSize - gInfo -> minRoomSize;
}

void Dungeon::MakeRoom()
{
	Cell &crrCell = tree.Get();
	SDL_FRect room = crrCell.space;
	SDL_FRect room2;

	const bool doubleRoom = (rand() % 100) < gInfo -> doubleRoomProb;

	float dX = RoundTo(room.w * ((gInfo -> minRoomSize + rand() % cache.deltaRoomSize) / 100.0f), gInfo -> tileSize);
	float dY = RoundTo(room.h * ((gInfo -> minRoomSize + rand() % cache.deltaRoomSize) / 100.0f), gInfo -> tileSize);

	room.w -= dX;
	room.h -= dY;

	if (doubleRoom)
	{
		room2 = room;

		if (dX > dY)
		{
			const float extra = RoundTo(dX * ((rand() % 100) / 100.0f), gInfo -> tileSize);

			room2.h = RoundTo(room2.h / 2.0f, gInfo -> tileSize);
			room2.w += extra;
			dX -= extra;

			if (rand() & 1) room2.y = room.y + room.h - room2.h;
		}
		else
		{
			const float extra = RoundTo(dY * ((rand() % 100) / 100.0f), gInfo -> tileSize);

			room2.w = RoundTo(room2.w / 2.0f, gInfo -> tileSize);
			room2.h += extra;
			dY -= extra;

			if (rand() & 1) room2.x = room.x + room.w - room2.w;
		}
	}

	const float xOffset = RoundTo(dX * (rand() % 100) / 100.0f, gInfo -> tileSize);
	const float yOffset = RoundTo(dY * (rand() % 100) / 100.0f, gInfo -> tileSize);

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

		space.x += cache.tileSize4;
		space.y += cache.tileSize4;
		space.w -= cache.tileSize5;
		space.h -= cache.tileSize5;

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

	randSize = RoundTo(randSize, gInfo -> tileSize);

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