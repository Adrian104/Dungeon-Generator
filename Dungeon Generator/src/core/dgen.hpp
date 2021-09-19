#pragma once
#include "utils.hpp"
#include "btree.hpp"

struct Cell;
struct Node;
struct Room;
struct GenInput;
struct GenOutput;
struct Generator;

struct Cell
{
	Rect space;
	Node *selNode;

	Cell() : space{}, selNode(nullptr) {}
	Cell(int w, int h) : space{ 0, 0, w, h }, selNode(nullptr) {}
};

struct Node
{
	enum class Type : byte { NORMAL, ENTRY, INTERNAL };
	static Node refNode;
	
	const Type type;
	unsigned int status;

	int gCost;
	int hCost;

	byte path;
	Point pos;

	Node *links[4];
	Node *prevNode;

	Node(const Type pType) : type(pType), status(0), gCost(0), hCost(0), path(0), pos{ 0, 0 }, links{ nullptr, nullptr, nullptr, nullptr }, prevNode(nullptr) {}
	Node(const Type pType, const int x, const int y) : type(pType), status(0), gCost(0), hCost(0), path(0), pos{ x, y }, links{ nullptr, nullptr, nullptr, nullptr }, prevNode(nullptr) {}
};

struct Room
{
	Node iNode;
	Node eNodes[4];
	std::vector<Rect> rects;

	Room() : iNode(Node::Type::INTERNAL), eNodes{ Node(Node::Type::ENTRY), Node(Node::Type::ENTRY), Node(Node::Type::ENTRY), Node(Node::Type::ENTRY) } {}
};

struct GenInput
{
	int xSize, ySize;
	int doubleRoomProb;
	int maxDepth, minDepth;
	int spaceSizeRandomness;
	int additionalConnections;
	int maxRoomSize, minRoomSize;
	int randAreaDens, randAreaProb, randAreaSize;
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
	int minSpaceSize;

	unsigned int statusCounter;

	GenInput *gInput;
	GenOutput *gOutput;

	uint32_t bValues;
	Uniforms uniforms;
	std::mt19937 mtEngine;

	bt::Node<Cell> *root;
	std::vector<Room> rooms;
	std::vector<std::pair<int, Node*>> openNodes;

	std::map<std::pair<int, int>, Node> posXNodes;
	std::map<std::pair<int, int>, Node*> posYNodes;
	
	void Clear();
	void Prepare();
	bool Validate();
	void LinkNodes();
	void FindPaths();
	void OptimizeNodes();
	void GenerateRooms();
	void GenerateOutput();
	void GenerateTree(bt::Node<Cell> &btNode, int left);

	bool RandomBool();
	Node &AddRegNode(int x, int y);
	void CreateSpaceNodes(Rect &space);
	void CreateRoomNodes(Rect &space, Room &room);
	Node *GetRandomNode(bt::Node<Cell> *const btNode);

	void MakeRoom(bt::Node<Cell> &btNode);
	void FindPath(bt::Node<Cell> &btNode);

	Generator();
	~Generator();

	void Generate(GenInput *genInput, GenOutput *genOutput, const uint32_t seed);
	void GenerateDebug(GenInput *genInput, GenOutput *genOutput, const uint32_t seed, Caller<void> &callback);
};