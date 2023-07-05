#pragma once

#include "bin_tree.hpp"
#include "rand.hpp"

#include <limits>
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
	static Node s_sentinel;

	int m_gCost = 0;
	int m_hCost = 0;

	uint8_t m_path = 0;
	uint8_t m_origin = 0;
	uint32_t m_status = 0;

	Point m_pos{};
	Node* m_links[4]{ &s_sentinel, &s_sentinel, &s_sentinel, &s_sentinel };

	Node() = default;
	Node(uint32_t status) : m_status(status) {}

	virtual Room* ToRoom() { return nullptr; }
};

struct Tag
{
	static constexpr uint64_t s_emptyIndex = (1ULL << 58) - 1;
	uint64_t m_pos = 0;

	struct Data
	{
		uint64_t m_index : 58;
		uint64_t m_origin : 2;
		uint64_t m_linkBits : 4;
	};

	union
	{
		Data m_data{};
		Node* m_node;
	};

	Tag();
	Tag(int high, int low);
	Tag(int high, int low, uint8_t linkBits, uint8_t origin, uint64_t index);
};

struct RadixSort
{
	void* const m_memory;

	RadixSort(const size_t maxSize);
	~RadixSort();

	void Sort(Tag* arr, const size_t size);
};

struct Room final : public Node
{
	std::vector<Rect> m_rects;
	int m_edges[4]{ std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() };

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

	std::vector<Tag> m_tags;
	std::vector<Node> m_nodes;
	std::vector<Room> m_rooms;
	bt::Node<Cell>* m_root = nullptr;

	static constexpr int s_roomSizeLimit = 4;

	void Clear();
	void Prepare();
	void FindPaths();
	void CreateNodes();
	void OptimizeNodes();
	void GenerateRooms();
	void GenerateOutput();

	uint32_t GenerateTree(bt::Node<Cell>& btNode, int left);
	static void DeleteTree(bt::Node<Cell>* btNode);

	void CreateSpaceTags(Rect& space);
	void CreateRoomTags(bt::Node<Cell>& btNode, Room& room);

	Generator() = default;
	~Generator() { Clear(); }

	void Generate(const GenInput* input, GenOutput* output);
};