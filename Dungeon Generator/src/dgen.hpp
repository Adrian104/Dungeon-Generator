#pragma once
#include <unordered_map>
#include <forward_list>
#include <cstdlib>
#include <time.h>
#include "vport.hpp"
#include "btree.hpp"

inline float RoundTo(float num, float step)
{
	const float div = num / step;
	const int i = int(div);

	return step * (i + ((div - float(i)) >= 0.5f));
}

inline bool IsInside(SDL_FRect &rect, SDL_FPoint &point)
{
	return rect.x < point.x && (rect.x + rect.w) > point.x && rect.y < point.y && (rect.y + rect.h) > point.y;
}

struct Room;
struct Cell;
struct GenInfo;
struct Dungeon;

typedef BTree<Cell> Tree;

struct Room
{
	Room *nextRoom;
	SDL_FRect room;

	Room() : nextRoom(nullptr), room{} {}
	~Room() { delete nextRoom; }
};

struct Cell
{
	Room *roomList;
	SDL_FRect space;

	Cell() : roomList(nullptr), space{} {}
	~Cell() { delete roomList; }
};

struct GenInfo
{
	float tileSize;
	int xSize, ySize;
	int doubleRoomProb;
	int maxDepth, minDepth;
	int spaceSizeRandomness;
	int maxRoomSize, minRoomSize;
};

struct Dungeon
{
	Tree tree;
	GenInfo *gInfo;

	void MakeRoom();
	void Divide(int left);

	Dungeon();
	~Dungeon();

	void Clear();
	void Generate(GenInfo *genInfo);
};