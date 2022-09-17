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
	bool locked = false;
	Rect space{};

	int roomOffset = std::numeric_limits<int>::max();
	int roomCount = 0;

	Cell() = default;
	Cell(int w, int h) : space(0, 0, w, h) {}
};

struct Node
{
	int gCost = 0;
	int hCost = 0;

	byte path = 0;
	byte origin = 0;
	uint status = 0;

	Point pos{};
	Node* links[4]{};

	Node() = default;
	Node(int x, int y) : pos(x, y) {}

	virtual Room* ToRoom() { return nullptr; }
};

struct Room : public Node
{
	std::vector<Rect> rects;
	int edges[4]{ std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() };

	Room() = default;
	Room(int x, int y) : Node(x, y) {}

	void ComputeEdges();
	Room* ToRoom() override { return this; }
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
	int extDist = 0;
	int intDist = 0;

	int roomCount = 0;
	int deltaDepth = 0;
	int targetDepth = 0;
	int minSpaceSize = 0;

	Random random;

	GenOutput* gOutput = nullptr;
	const GenInput* gInput = nullptr;

	std::vector<Room> rooms;
	bt::Node<Cell>* root = nullptr;

	std::map<std::pair<int, int>, Node> nodes;
	std::uniform_real_distribution<float> uniSpace;

	static constexpr int roomSizeLimit = 4;

	void Clear();
	void Prepare();
	void LinkNodes();
	void FindPaths();
	void OptimizeNodes();
	void GenerateRooms();
	void GenerateOutput();
	void GenerateTree(bt::Node<Cell>& btNode, int left);

	Node& RegisterNode(int x, int y);
	void CreateSpaceNodes(Rect& space);
	void CreateRoomNodes(Rect& space, Room& room);

	Generator() = default;
	~Generator() { Clear(); }

	void Generate(const GenInput* genInput, GenOutput* genOutput);
};