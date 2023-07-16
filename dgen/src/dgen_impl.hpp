#pragma once

#include "../include/dgen/dgen.hpp"
#include "bin_tree.hpp"
#include "rand.hpp"

#include <limits>
#include <vector>

namespace dg::impl
{
	struct Room;
	enum Dir { NORTH, EAST, SOUTH, WEST };

	struct Cell
	{
		enum Flag : uint32_t { RANDOM_PATH, SPARSE_AREA, GENERATE_ROOMS, CONNECT_ROOMS };

		uint32_t m_flags = 0;
		Rect m_space{};

		int m_roomOffset = std::numeric_limits<int>::max();
		int m_roomCount = 0;

		Cell() = default;
		Cell(int w, int h) : m_space(0, 0, w, h) {}
	};

	struct Vertex
	{
		static Vertex s_sentinel;

		float m_gcost = 0;
		float m_hcost = 0;

		uint8_t m_path = 0;
		uint8_t m_origin = 0;
		uint32_t m_status = 0;

		Point m_pos{};
		Vertex* m_links[4]{ &s_sentinel, &s_sentinel, &s_sentinel, &s_sentinel };

		Vertex() = default;
		Vertex(uint32_t status) : m_status(status) {}

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
			Vertex* m_vertex;
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

	struct Room final : public Vertex
	{
		Node<Cell>& m_node;
		Point m_entrances[4]{};
		size_t m_rectBegin = 0;
		size_t m_rectEnd = 0;

		Room(Node<Cell>& node) : m_node(node) {}
		Room* ToRoom() override { return this; }
	};

	struct Generator
	{
		int m_spaceOffset = 0;
		int m_spaceShrink = 0;

		int m_deltaDepth = 0;
		int m_targetDepth = 0;
		int m_minSpaceSize = 0;
		int m_randPathDepth = 0;
		float m_minSpaceRand = 0;

		int m_totalRoomCount = 0;
		int m_partialPathCount = 0;

		Random m_random;

		Output* m_output = nullptr;
		const Input* m_input = nullptr;

		std::vector<Tag> m_tags;
		std::vector<Room> m_rooms;
		std::vector<Vertex> m_vertices;
		Node<Cell>* m_rootNode = nullptr;

		static constexpr int s_roomSizeLimit = 4;

		void Clear();
		void Prepare();
		void FindPaths();
		void CreateVertices();
		void OptimizeVertices();
		void GenerateRooms();
		void GenerateOutput();

		uint32_t GenerateTree(Node<Cell>& node, int left);
		static void DeleteTree(Node<Cell>* node);
		static int GetNearestRoomTo(const Point point, Node<Cell>* node);

		Generator() = default;
		~Generator() { Clear(); }

		void Generate(const Input* input, Output* output);
	};
}