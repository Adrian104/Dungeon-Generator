#pragma once

#include "btree.hpp"
#include "rand.hpp"

#include <limits>
#include <map>
#include <vector>

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

struct Room;
using Vec = Point;
enum Dir { NORTH, EAST, SOUTH, WEST };

struct Cell
{
	enum Flag : uint32_t { SPARSE_AREA, GENERATE_ROOMS, CONNECT_ROOMS };

	uint32_t m_flags = 0;
	Rect m_space{};

	int m_roomOffset = std::numeric_limits<int>::max();
	int m_roomCount = 0;

	Cell() = default;
	Cell(int w, int h) : m_space(0, 0, w, h) {}
};

struct Node
{
	static Node sentinel;

	int m_gCost = 0;
	int m_hCost = 0;

	uint8_t m_path = 0;
	uint8_t m_origin = 0;
	uint32_t m_status = 0;

	Point m_pos{};
	Node* m_links[4]{ &sentinel, &sentinel, &sentinel, &sentinel };

	Node() = default;
	Node(int x, int y) : m_pos(x, y) {}
	Node(uint32_t status) : m_status(status) {}

	virtual Room* ToRoom() { return nullptr; }
};

struct Room : public Node
{
	std::vector<Rect> m_rects;
	int m_edges[4]{ std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() };

	Room() = default;
	Room(int x, int y) : Node(x, y) {}

	void ComputeEdges();
	Room* ToRoom() override { return this; }
};

struct GenInput
{
	uint64_t m_seed;
	bool m_generateFewerPaths;

	int m_sparseAreaDepth;
	int m_width, m_height;
	int m_spaceInterdistance;
	int m_minDepth, m_maxDepth;

	float m_doubleRoomProb;
	float m_heuristicFactor;
	float m_spaceSizeRandomness;
	float m_minRoomSize, m_maxRoomSize;
	float m_sparseAreaDens, m_sparseAreaProb;
};

struct GenOutput
{
	std::vector<Rect> m_rooms;
	std::vector<Point> m_entrances;
	std::vector<std::pair<Point, Vec>> m_paths;
};

struct Generator
{
	int m_spaceOffset = 0;
	int m_spaceShrink = 0;

	int m_deltaDepth = 0;
	int m_targetDepth = 0;
	int m_minSpaceSize = 0;
	float m_minSpaceRand = 0;

	int m_totalRoomCount = 0;
	int m_partialPathCount = 0;

	Random m_random;

	GenOutput* m_output = nullptr;
	const GenInput* m_input = nullptr;

	std::vector<Room> m_rooms;
	bt::Node<Cell>* m_root = nullptr;
	std::map<std::pair<int, int>, Node> m_nodes;

	static constexpr int roomSizeLimit = 4;

	void Clear();
	void Prepare();
	void LinkNodes();
	void FindPaths();
	void OptimizeNodes();
	void GenerateRooms();
	void GenerateOutput();

	uint32_t GenerateTree(bt::Node<Cell>& btNode, int left);
	static void DeleteTree(bt::Node<Cell>* btNode);

	Node& RegisterNode(int x, int y);
	void CreateSpaceNodes(Rect& space);
	void CreateRoomNodes(Rect& space, Room& room);

	Generator() = default;
	~Generator() { Clear(); }

	void Generate(const GenInput* input, GenOutput* output);
};