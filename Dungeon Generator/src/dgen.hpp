#pragma once
#include <map>
#include <vector>
#include <random>
#include <forward_list>
#include "macros.hpp"
#include "vport.hpp"
#include "btree.hpp"

enum Dir : uint8_t { NORTH, EAST, SOUTH, WEST, INVALID };

inline SDL_FPoint ToFPoint(const SDL_Point &point) { return { float(point.x), float(point.y) }; }
inline SDL_FRect ToFRect(const SDL_Rect &rect) { return { float(rect.x), float(rect.y), float(rect.w), float(rect.h) }; }

struct Room;
struct Cell;
struct PNode;
struct Dungeon;
struct GenInput;

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
	PNode *internalNode;

	Cell() : space{}, roomList(nullptr), internalNode(nullptr) {}
	~Cell() { delete roomList; }
};

struct PNode
{
	static PNode null;
	static PNode *stop;

	static std::vector<std::pair<int, PNode*>> *heap;

	enum : uint8_t { UNVISITED, OPEN, CLOSED };
	enum : uint8_t { E_NODE = 1 << 4, I_NODE = 1 << 5 };

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

struct GenInput
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

		std::uniform_int_distribution<int> uni0to99;
		std::uniform_int_distribution<int> uniDepth;
		std::uniform_real_distribution<float> uni0to1f;
		std::uniform_real_distribution<float> uniRoom;
		std::uniform_real_distribution<float> uniSpace;
	};

	Tree tree;
	Cache cache;
	GenInput *gInput;

	uint32_t seed;
	std::mt19937 mtEngine;
	std::random_device rd;

	std::forward_list<PNode> pNodes;
	std::multimap<int, PNode*> pXNodes;
	std::multimap<int, PNode*> pYNodes;

	std::vector<PNode*> usedNodes;
	std::vector<std::pair<int, PNode*>> openNodes;
	
	void MakeRoom();
	void LinkNodes();
	void FindPaths();
	void CreateNodes();
	bool Divide(int left);
	void Prepare(const bool newSeed);

	PNode &AddNode(int x, int y);

	template <uint8_t dir>
	void AddEntryNode(Cell &cell);

	Dungeon();
	~Dungeon();

	void Clear();
	void Generate(GenInput *genInput, const bool newSeed = true);
};