#pragma once

#include "btree.hpp"
#include "rand.hpp"

#include <limits>
#include <map>
#include <random>
#include <vector>

struct Room;

struct Point
{
	int x, y;

	Point() = default;
	Point(int pX, int pY) : x(pX), y(pY) {}
};

struct Rect
{
	int x, y, w, h;

	Rect() = default;
	Rect(int pX, int pY, int pW, int pH) : x(pX), y(pY), w(pW), h(pH) {}
};

enum Dir { NORTH, EAST, SOUTH, WEST };

using Vec = Point;
using uint = unsigned int;
using byte = unsigned char;

struct Cell
{
	bool locked;
	Rect space;
	Room *room;

	Cell() : locked(false), space(), room(nullptr) {}
	Cell(int w, int h) : locked(false), space(0, 0, w, h), room(nullptr) {}
};

struct Node
{
	int gCost;
	int hCost;

	byte path;
	byte origin;

	Point pos;
	uint status;
	Node *links[4];

	Node() : gCost(0), hCost(0), path(0), origin(0), pos{ 0, 0 }, status(0), links{ nullptr, nullptr, nullptr, nullptr } {}
	Node(int x, int y) : gCost(0), hCost(0), path(0), origin(0), pos{ x, y }, status(0), links{ nullptr, nullptr, nullptr, nullptr } {}

	virtual Room *ToRoom() { return nullptr; }
};

struct Room : public Node
{
	int edges[4];
	std::vector<Rect> rects;

	Room() : Node(), edges{ std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() } {}
	Room(int x, int y) : Node(x, y), edges{ std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() } {}

	Room *ToRoom() override { return this; }
};

struct GenInput
{
	Random::seed_type m_seed;
	bool m_generateFewerPaths;

	int m_randAreaDepth;
	int m_width, m_height;
	int m_spaceInterdistance;
	int m_minDepth, m_maxDepth;

	float m_doubleRoomProb;
	float m_heuristicFactor;
	float m_spaceSizeRandomness;
	float m_minRoomSize, m_maxRoomSize;
	float m_randAreaDens, m_randAreaProb;
};

struct GenOutput
{
	std::vector<Rect> m_rooms;
	std::vector<Point> m_entrances;
	std::vector<std::pair<Point, Vec>> m_paths;
};

struct Generator
{
	int extDist;
	int intDist;

	int roomCount;
	int deltaDepth;
	int targetDepth;
	int minSpaceSize;

	GenOutput *gOutput;
	const GenInput *gInput;

	Random random;

	bt::Node<Cell> *root;
	std::vector<Room> rooms;

	std::map<std::pair<int, int>, Node> posXNodes;
	std::map<std::pair<int, int>, Node*> posYNodes;
	std::uniform_real_distribution<float> uniSpace;
	
	static constexpr int roomSizeLimit = 4;

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

	Generator();
	~Generator();

	void Generate(const GenInput *genInput, GenOutput *genOutput);
};