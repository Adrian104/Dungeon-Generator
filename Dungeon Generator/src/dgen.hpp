#pragma once
#include <vector>
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

inline SDL_FPoint ToFPoint(const SDL_Point &point) { return { float(point.x), float(point.y) }; }
inline SDL_FRect ToFRect(const SDL_Rect &rect) { return { float(rect.x), float(rect.y), float(rect.w), float(rect.h) }; }

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
	bool horizontal;
	SDL_Rect space;
	Room *roomList;
	PNode *entryNode;

	Cell() : horizontal(false), space{}, roomList(nullptr), entryNode(nullptr) {}
	~Cell() { delete roomList; }
};

struct PNode
{
	static PNode null;
	static PNode *stop;
	static std::vector<std::pair<int, PNode*>> *heap;

	enum : uint8_t { UNVISITED, OPEN, CLOSED };
	enum : uint8_t { NORTH, EAST, SOUTH, WEST };

	int gCost;
	int hCost;
	int fCost;

	uint8_t mode;
	uint8_t path;

	PNode *links[4];
	PNode *prevNode;
	const SDL_Point pos;

	PNode(const SDL_Point &pPos) : gCost(0), hCost(0), fCost(0), mode(UNVISITED), path(0), links{ &null, &null, &null, &null }, prevNode(nullptr), pos(pPos) {}
	
	void Reset();
	void Open(PNode *prev);
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

	std::vector<PNode*> usedNodes;
	std::vector<std::pair<int, PNode*>> openNodes;

	std::forward_list<PNode> pNodes;
	std::unordered_multimap<int, PNode*> *pXNodes;
	std::unordered_multimap<int, PNode*> *pYNodes;
	
	void Prepare();
	void MakeRoom();
	void LinkNodes();
	void FindPaths();
	void CreateNodes();
	bool Divide(int left);

	PNode &AddNode(int x, int y);

	Dungeon();
	~Dungeon();

	void Clear();
	void Generate(GenInfo *genInfo);
};