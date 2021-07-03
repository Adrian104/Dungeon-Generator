#pragma once
#include "utils.hpp"
#include "btree.hpp"

struct Cell;
struct Node;
struct GenInput;
struct GenOutput;
struct Generator;

enum Dir : byte { NORTH, EAST, SOUTH, WEST, INVALID };

struct Cell
{
	Rect space;
	Node *selNode;
	std::vector<Rect> roomVec;

	Cell() : space{}, selNode(nullptr) {}
	Cell(int w, int h) : space{ 0, 0, w, h }, selNode(nullptr) {}
};

struct Node
{
	static Node *stop;
	static Node reqLink;

	static std::vector<std::pair<int, Node*>> *heap;

	enum class Mode : byte { UNVISITED, OPEN, CLOSED };
	enum class Type : byte { NORMAL, ENTRY, INTERNAL };

	const Type type;

	int gCost;
	int hCost;
	int fCost;

	Mode mode;
	Point pos;
	byte path;

	Node *links[4];
	Node *prevNode;

	Node(const Type pType) : type(pType), gCost(0), hCost(0), fCost(0), mode(Mode::UNVISITED), pos{ 0, 0 }, path(0), links{ nullptr, nullptr, nullptr, nullptr }, prevNode(nullptr) {}
	Node(const Type pType, const int x, const int y) : type(pType), gCost(0), hCost(0), fCost(0), mode(Mode::UNVISITED), pos{ x, y }, path(0), links{ nullptr, nullptr, nullptr, nullptr }, prevNode(nullptr) {}
	
	void Reset();
	void Open(Node *prev);

	inline bool CheckIfGCost() const { return path == 0 && type == Type::NORMAL; }
	inline bool CheckIfEntrance() const { return path != 0 && type == Type::ENTRY; }
	inline bool CheckIfPath(const byte dir) const { return (path & (1 << dir)) != 0 ? (links[dir] -> type != Type::INTERNAL) : false; }
};

struct GenInput
{
	int xSize, ySize;
	int doubleRoomProb;
	int maxDepth, minDepth;
	int spaceSizeRandomness;
	int maxRoomSize, minRoomSize;
};

struct GenOutput
{
	std::vector<Rect> rooms;
	std::vector<Point> entrances;
	std::vector<std::pair<Point, Vec>> paths;
};

struct Generator
{
	struct Uniforms
	{
		std::uniform_int_distribution<int> uni0to99;
		std::uniform_int_distribution<int> uniDepth;
		std::uniform_real_distribution<float> uniRoom;
		std::uniform_real_distribution<float> uniSpace;
		std::uniform_real_distribution<float> uni0to1f;
	};

	int roomCount;
	int deltaDepth;

	GenInput *gInput;
	GenOutput *gOutput;

	uint32_t bValues;
	Uniforms uniforms;
	std::mt19937 mtEngine;

	bt::Node<Cell> *root;

	std::forward_list<Node> nodes;
	std::map<std::pair<int, int>, Node*> posXNodes;
	std::map<std::pair<int, int>, Node*> posYNodes;

	std::vector<Node*> usedNodes;
	std::vector<std::pair<int, Node*>> openNodes;
	
	void Prepare();
	void LinkNodes();
	void GenerateOutput();
	bool Divide(bt::Node<Cell> &btNode, int left);

	bool RandomBool();
	Node &AddRegNode(int x, int y);
	void CreateRoomNodes(Cell &cell);
	void CreateSpaceNodes(Cell &cell);

	void MakeRoom(bt::Node<Cell> &btNode);
	void FindPath(bt::Node<Cell> &btNode);

	Generator();
	~Generator();

	void Clear();
	void Generate(GenInput *genInput, GenOutput *genOutput, const uint32_t seed);
	void GenerateDebug(GenInput *genInput, GenOutput *genOutput, const uint32_t seed, Caller<void> &callback);
};