#pragma once
#include <map>
#include <vector>
#include <random>
#include <forward_list>
#include "global.hpp"
#include "vport.hpp"
#include "btree.hpp"

struct Cell;
struct Node;
struct Dungeon;
struct GenInput;

typedef BinTree<Cell> Tree;
enum Dir : uint8_t { NORTH, EAST, SOUTH, WEST, INVALID };

struct Cell
{
	SDL_Rect space;
	Node *internalNode;
	std::forward_list<SDL_Rect> roomList;

	Cell() : space{}, internalNode(nullptr) {}
};

struct Node
{
	static Node null;
	static Node *stop;

	static std::vector<std::pair<int, Node*>> *heap;

	enum : uint8_t { UNVISITED, OPEN, CLOSED };
	enum : uint8_t { E_NODE = 1 << 4, I_NODE = 1 << 5 };

	int gCost;
	int hCost;
	int fCost;

	uint8_t mode;
	uint8_t path;

	Node *links[4];
	Node *prevNode;
	const SDL_Point pos;

	Node(const int x, const int y) : gCost(0), hCost(0), fCost(0), mode(UNVISITED), path(0), links{ &null, &null, &null, &null }, prevNode(nullptr), pos{ x, y } {}
	
	void Reset();
	void Open(Node *prev);
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
	struct Uniforms
	{
		std::uniform_int_distribution<int> uni0to99;
		std::uniform_int_distribution<int> uniDepth;
		std::uniform_real_distribution<float> uniRoom;
		std::uniform_real_distribution<float> uniSpace;
		std::uniform_real_distribution<float> uni0to1f;
	};

	int deltaDepth;

	Tree tree;
	GenInput *gInput;
	Uniforms uniforms;

	uint32_t bValues;
	std::mt19937 mtEngine;

	std::forward_list<Node> nodes;
	std::map<std::pair<int, int>, Node*> posXNodes;
	std::map<std::pair<int, int>, Node*> posYNodes;

	std::vector<Node*> usedNodes;
	std::vector<std::pair<int, Node*>> openNodes;
	
	void Prepare();
	void MakeRoom();
	void FindPath();
	void LinkNodes();
	bool Divide(int left);

	bool RandomBool();
	void CreateRoomNodes();
	void CreateSpaceNodes();
	Node &AddRegNode(int x, int y);

	Dungeon();
	~Dungeon();

	void Clear();
	void Generate(GenInput *genInput, const uint32_t seed);
};