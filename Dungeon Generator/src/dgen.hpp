#pragma once
#include <unordered_map>
#include <forward_list>
#include <cstdlib>
#include <time.h>
#include "macros.hpp"
#include "vport.hpp"
#include "btree.hpp"

inline bool IsInside(SDL_Rect &rect, SDL_Point &point)
{
	return rect.x < point.x && (rect.x + rect.w) > point.x && rect.y < point.y && (rect.y + rect.h) > point.y;
}

inline SDL_FPoint ToFPoint(SDL_Point &point) { return { float(point.x), float(point.y) }; }
inline SDL_FRect ToFRect(SDL_Rect &rect) { return { float(rect.x), float(rect.y), float(rect.w), float(rect.h) }; }

struct Room;
struct Cell;
struct PNode;
struct GenInfo;
struct Dungeon;

typedef BinTree<Cell> Tree;

struct Room
{
	SDL_Rect room;
	Room *nextRoom;

	Room() : room{}, nextRoom(nullptr) {}
	Room(SDL_Rect &rect) : room(rect), nextRoom(nullptr) {}
	~Room() { delete nextRoom; }
};

struct Cell
{
	SDL_Rect space;
	Room *roomList;
	PNode *entryNode;

	Cell() : space{}, roomList(nullptr), entryNode(nullptr) {}
	~Cell() { delete roomList; }
};

struct PNode
{
	static PNode null;
	enum : uint8_t { NORTH, EAST, SOUTH, WEST };

	SDL_Point pos;
	PNode *links[4];

	PNode(const SDL_Point &pPos) : pos(pPos), links{ &null, &null, &null, &null } {}
};

struct GenInfo
{
	int xSize, ySize;
	int doubleRoomProb;
	int maxDepth, minDepth;
	int spaceSizeRandomness;
	int maxRoomSize, minRoomSize;
};

struct Dungeon
{
	struct Cache
	{
		int deltaDepth;
		int deltaRoomSize;
	};

	Tree tree;
	Cache cache;
	GenInfo *gInfo;

	std::forward_list<PNode> pNodes;
	std::unordered_multimap<int, PNode*> *pXNodes;
	std::unordered_multimap<int, PNode*> *pYNodes;
	
	void Prepare();
	void MakeRoom();
	void LinkNodes();
	void CreateNodes();
	bool Divide(int left);

	PNode &AddNode(int x, int y);

	Dungeon();
	~Dungeon();

	void Clear();
	void Generate(GenInfo *genInfo);
};