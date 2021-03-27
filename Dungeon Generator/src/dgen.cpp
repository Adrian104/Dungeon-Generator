#include "dgen.hpp"

#ifndef _DEBUG
#define FULL_SCREEN
#endif

Dungeon::Dungeon() : gInfo(nullptr) {}
Dungeon::~Dungeon() { Clear(); }

void Dungeon::MakeRoom()
{
	Cell &crrCell = tree.Get();
	SDL_FRect room = crrCell.space;
	SDL_FRect room2;

	const int maxmin = gInfo -> maxRoomSize - gInfo -> minRoomSize;
	const bool doubleRoom = (rand() % 100) < gInfo -> doubleRoomProb;

	float dX = RoundTo(room.w * ((gInfo -> minRoomSize + rand() % maxmin) / 100.0f), gInfo -> tileSize);
	float dY = RoundTo(room.h * ((gInfo -> minRoomSize + rand() % maxmin) / 100.0f), gInfo -> tileSize);

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

		const float xyOffset = gInfo -> tileSize * 4;
		const float whOffset = gInfo -> tileSize * 5;

		space.x += xyOffset;
		space.y += xyOffset;
		space.w -= whOffset;
		space.h -= whOffset;

		MakeRoom();
		return;
	}
	else if (int delta = gInfo -> maxDepth - gInfo -> minDepth; left <= delta)
	{
		if (rand() % (delta + 1) >= left) goto nomore;
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

	gInfo -> xSize = int(RoundTo(float(gInfo -> xSize), gInfo -> tileSize));
	gInfo -> ySize = int(RoundTo(float(gInfo -> ySize), gInfo -> tileSize));

	tree.Get().space.w = float(gInfo -> xSize) - 3 * gInfo -> tileSize;
	tree.Get().space.h = float(gInfo -> ySize) - 3 * gInfo -> tileSize;

	Divide(gInfo -> maxDepth);
}