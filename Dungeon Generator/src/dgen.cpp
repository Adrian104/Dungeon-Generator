#include "dgen.hpp"

#ifndef _DEBUG
#define FULL_SCREEN
#endif

Dungeon::Dungeon() : gInfo(nullptr) {}
Dungeon::~Dungeon() { Clear(); }

void Dungeon::MakeRoom()
{
	// ---------------

	SDL_FRect &space = tree.Get().space;

	space.x += 2;
	space.y += 2;
	space.w -= 4;
	space.h -= 4;

	// ---------------

	Cell &crrCell = tree.Get();
	SDL_FRect room = crrCell.space;
	SDL_FRect room2;

	const int maxmin = gInfo -> maxRoomSize - gInfo -> minRoomSize;
	const bool doubleRoom = (rand() % 100) < gInfo -> doubleRoomProb;

	float dX = room.w * ((gInfo -> minRoomSize + rand() % maxmin) / 100.0f);
	float dY = room.h * ((gInfo -> minRoomSize + rand() % maxmin) / 100.0f);

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

			if (rand() & 1) room2.y += room2.h;
		}
		else
		{
			const float extra = dY * ((rand() % 100) / 100.0f);

			room2.w /= 2.0f;
			room2.h += extra;
			dY -= extra;

			if (rand() & 1) room2.x += room2.w;
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

	tree.Get().space.w = float(gInfo -> xSize);
	tree.Get().space.h = float(gInfo -> ySize);

	Divide(gInfo -> maxDepth);
}