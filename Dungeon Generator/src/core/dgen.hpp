#pragma once
#include "utils.hpp"
#include "btree.hpp"
#include "heap.hpp"
#include "rand.hpp"

struct Cell;
struct Node;
struct Room;
struct GenInput;
struct GenOutput;
struct Generator;

struct Cell
{
	Rect space;
	Room *room;

	Cell() : space{}, room(nullptr) {}
	Cell(int w, int h) : space{ 0, 0, w, h }, room(nullptr) {}
};

struct Node
{
	int gCost;
	int hCost;

	byte path;
	Point pos;
	uint status;

	Node *links[4];
	Node *prevNode;

	Node() : gCost(0), hCost(0), path(0), pos{ 0, 0 }, status(0), links{ nullptr, nullptr, nullptr, nullptr }, prevNode(nullptr) {}
	Node(int x, int y) : gCost(0), hCost(0), path(0), pos{ x, y }, status(0), links{ nullptr, nullptr, nullptr, nullptr }, prevNode(nullptr) {}

	virtual Room *ToRoom() { return nullptr; }
};

struct Room : public Node
{
	static Room flag;

	int edges[4];
	std::vector<Rect> rects;

	Room() : Node(), edges{ std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() } {}
	Room(int x, int y) : Node(x, y), edges{ std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() } {}

	Room *ToRoom() override { return this; }
};

struct GenInput
{
	int xSize, ySize;
	int randAreaDepth;
	int maxDepth, minDepth;
	int additionalConnections;

	float doubleRoomProb;
	float heuristicFactor;
	float spaceSizeRandomness;
	float maxRoomSize, minRoomSize;
	float randAreaDens, randAreaProb;
};

struct GenOutput
{
	std::vector<Rect> rooms;
	std::vector<Point> entrances;
	std::vector<std::pair<Point, Vec>> paths;
};

struct Generator
{
	int roomCount;
	int deltaDepth;
	int targetDepth;
	int minSpaceSize;

	uint statusCounter;

	GenOutput *gOutput;
	const GenInput *gInput;

	Random random;
	Heap<int, Node*> heap;

	bt::Node<Cell> *root;
	std::vector<Room> rooms;

	std::map<std::pair<int, int>, Node> posXNodes;
	std::map<std::pair<int, int>, Node*> posYNodes;

	std::uniform_real_distribution<float> uniRoom;
	std::uniform_real_distribution<float> uniSpace;
	
	void Clear();
	void Prepare();
	void LinkNodes();
	void FindPaths();
	void OptimizeNodes();
	void GenerateRooms();
	void GenerateOutput();
	void GenerateTree(bt::Node<Cell> &btNode, int left);

	Node &AddRegNode(int x, int y);
	void CreateSpaceNodes(Rect &space);
	void CreateRoomNodes(Rect &space, Room &room);
	Room *GetRandomRoom(bt::Node<Cell> *const btNode);

	void MakeRoom(bt::Node<Cell> &btNode);
	void FindPath(bt::Node<Cell> &btNode);

	Generator();
	~Generator();

	void Generate(const GenInput *genInput, GenOutput *genOutput, const Random::seed_type seed);
	void GenerateDebug(const GenInput *genInput, GenOutput *genOutput, const Random::seed_type seed, Caller<void> &callback);
};