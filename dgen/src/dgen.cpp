#include "dgen.hpp"
#include "heap.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

Node Node::sentinel(std::numeric_limits<decltype(Node::m_status)>::max());

void Room::ComputeEdges()
{
	for (const Rect& rect : m_rects)
	{
		const int xPlusW = rect.x + rect.w;
		const int yPlusH = rect.y + rect.h;

		if (m_pos.x >= rect.x && m_pos.x < xPlusW)
		{
			if (m_edges[Dir::NORTH] > rect.y) m_edges[Dir::NORTH] = rect.y;
			if (m_edges[Dir::SOUTH] < yPlusH) m_edges[Dir::SOUTH] = yPlusH;
		}

		if (m_pos.y >= rect.y && m_pos.y < yPlusH)
		{
			if (m_edges[Dir::WEST] > rect.x) m_edges[Dir::WEST] = rect.x;
			if (m_edges[Dir::EAST] < xPlusW) m_edges[Dir::EAST] = xPlusW;
		}
	}

	m_edges[Dir::EAST]--;
	m_edges[Dir::SOUTH]--;
}

void Generator::Clear()
{
	m_nodes.clear();
	m_rooms.clear();

	if (m_root != nullptr)
	{
		DeleteTree(m_root);

		delete m_root;
		m_root = nullptr;
	}
}

void Generator::Prepare()
{
	if (m_input -> m_maxRoomSize <= 0)
		throw std::runtime_error("Variable 'maxRoomSize' is not a positive number");

	*m_output = {};
	m_random.Seed(m_input -> m_seed);

	m_spaceOffset = m_input -> m_spaceInterdistance + 1;
	m_spaceShrink = (m_spaceOffset << 1) - 1;

	m_minSpaceSize = static_cast<int>(roomSizeLimit / m_input -> m_maxRoomSize) + m_spaceShrink;
	m_minSpaceRand = (1.0f - m_input -> m_spaceSizeRandomness) * 0.5f;

	if (m_input -> m_width <= m_minSpaceSize || m_input -> m_height <= m_minSpaceSize)
		throw std::runtime_error("Root node is too small");

	m_root = new bt::Node<Cell>(nullptr, Cell(m_input -> m_width - 1, m_input -> m_height - 1));

	m_targetDepth = 0;
	m_totalRoomCount = 0;
	m_partialPathCount = 0;

	m_deltaDepth = m_input -> m_maxDepth - m_input -> m_minDepth;
}

void Generator::LinkNodes()
{
	struct Entry
	{
		int m_x; int m_y; Node* m_node;
		Entry(std::pair<int, int> pos, Node* node) : m_x(pos.first), m_y(pos.second), m_node(node) {}
	};

	if (m_nodes.empty())
		return;

	std::vector<Entry> cache;
	cache.reserve(m_nodes.size());

	Node* secondary = &(m_nodes.begin() -> second);
	for (auto& [pos, node] : m_nodes)
	{
		Node* const primary = &node;
		if (primary -> m_path & (1 << Dir::NORTH))
		{
			primary -> m_links[Dir::NORTH] = secondary;
			secondary -> m_links[Dir::SOUTH] = primary;
		}

		cache.emplace_back(pos, primary);
		secondary = primary;
	}

	auto cmp = [](const Entry& a, const Entry& b) -> bool { return a.m_y < b.m_y || (a.m_y == b.m_y && a.m_x < b.m_x); };
	std::sort(cache.begin(), cache.end(), cmp);

	for (Entry& entry : cache)
	{
		Node* const primary = entry.m_node;
		if (primary -> m_path & (1 << Dir::WEST))
		{
			primary -> m_links[Dir::WEST] = secondary;
			secondary -> m_links[Dir::EAST] = primary;
		}

		primary -> m_path = 0;
		secondary = primary;
	}
}

void Generator::FindPaths()
{
	uint32_t statusCounter = 1;
	MinHeap<int, Node*> heap;

	bt::Node<Cell>::defaultTraversal = bt::Traversal::POSTORDER;
	for (auto& btNode : *m_root)
	{
		if ((btNode.m_flags & (1 << Cell::Flag::CONNECT_ROOMS)) == 0)
			continue;

		int leftIndex = btNode.m_left -> m_roomOffset;
		const int leftCount = btNode.m_left -> m_roomCount;

		int rightIndex = btNode.m_right -> m_roomOffset;
		const int rightCount = btNode.m_right -> m_roomCount;

		switch (leftCount)
		{
		case 1: break;
		case 2: leftIndex += static_cast<int>(m_random.GetBit()); break;
		default: leftIndex += m_random.Get32() % leftCount;
		}

		switch (rightCount)
		{
		case 1: break;
		case 2: rightIndex += static_cast<int>(m_random.GetBit()); break;
		default: rightIndex += m_random.Get32() % rightCount;
		}

		Room* const start = m_rooms.data() + leftIndex;
		Room* const stop = m_rooms.data() + rightIndex;

		Node* crrNode = start;
		start -> m_gCost = 0;

		do
		{
			crrNode -> m_status = statusCounter + 1;
			Room* const crrRoom = crrNode -> ToRoom();

			for (int i = 0; i < 4; i++)
			{
				Node* const nNode = crrNode -> m_links[i];
				if (nNode -> m_status > statusCounter)
					continue;

				int newGCost = crrNode -> m_gCost;
				if ((crrNode -> m_path & (1 << i)) == 0)
				{
					int diff;
					int Point::*const axis = static_cast<bool>(i & 1) ? &Point::x : &Point::y;

					if (crrRoom == nullptr)
					{
						Room* const nRoom = nNode -> ToRoom();

						diff = crrNode -> m_pos.*axis;
						diff -= (nRoom != nullptr) ? (nRoom -> m_edges[i ^ 0b10]) : (nNode -> m_pos.*axis);
					}
					else diff = nNode -> m_pos.*axis - crrRoom -> m_edges[i];

					newGCost += diff < 0 ? -diff : diff;
				}

				if (nNode -> m_status < statusCounter)
				{
					const int dx = stop -> m_pos.x - nNode -> m_pos.x;
					const int dy = stop -> m_pos.y - nNode -> m_pos.y;

					const float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));

					nNode -> m_hCost = static_cast<int>(dist * m_input -> m_heuristicFactor);
					nNode -> m_status = statusCounter;

					goto add_to_heap;
				}

				if (newGCost < nNode -> m_gCost)
				{
					add_to_heap:
					nNode -> m_origin = i;
					nNode -> m_gCost = newGCost;

					heap.Push(newGCost + nNode -> m_hCost, nNode);
				}
			}

			do
			{
				crrNode = heap.TopObject();
				heap.Pop();

			} while (crrNode -> m_status > statusCounter);

		} while (crrNode != stop);

		statusCounter += 2;
		heap.Clear();

		do
		{
			const uint8_t origin = crrNode -> m_origin;
			const uint8_t realOrigin = origin ^ 0b10;

			crrNode -> m_path |= 1 << realOrigin;
			crrNode = crrNode -> m_links[realOrigin];
			crrNode -> m_path |= 1 << origin;

		} while (crrNode != start);
	}
}

void Generator::OptimizeNodes()
{
	auto iter = m_nodes.begin();
	const auto endIter = m_nodes.end();

	const uint8_t maskEW = m_input -> m_generateFewerPaths ? 0b1010 : 0b1111;
	const uint8_t maskNS = m_input -> m_generateFewerPaths ? 0b0101 : 0b1111;

	while (iter != endIter)
	{
		uint8_t& path = iter -> second.m_path;
		Node** links = iter -> second.m_links;

		if (path == 0)
		{
			zero:
			for (int i = 0; i < 4; i++)
				links[i] -> m_links[i ^ 0b10] = &Node::sentinel;

			iter = m_nodes.erase(iter);
			continue;
		}

		if ((path & maskEW) == 0b1010)
		{
			Node* const east = links[Dir::EAST];
			Node* const west = links[Dir::WEST];

			if (east -> ToRoom() == nullptr || west -> ToRoom() == nullptr)
			{
				east -> m_links[Dir::WEST] = west;
				west -> m_links[Dir::EAST] = east;

				links[Dir::EAST] = &Node::sentinel;
				links[Dir::WEST] = &Node::sentinel;

				if (path &= ~0b1010; path == 0)
					goto zero;
			}
		}

		if ((path & maskNS) == 0b0101)
		{
			Node* const north = links[Dir::NORTH];
			Node* const south = links[Dir::SOUTH];

			if (north -> ToRoom() == nullptr || south -> ToRoom() == nullptr)
			{
				north -> m_links[Dir::SOUTH] = south;
				south -> m_links[Dir::NORTH] = north;

				links[Dir::NORTH] = &Node::sentinel;
				links[Dir::SOUTH] = &Node::sentinel;

				if (path &= ~0b0101; path == 0)
					goto zero;
			}
		}

		m_partialPathCount += ((path >> Dir::NORTH) & 1) + ((path >> Dir::EAST) & 1);
		iter++;
	}
}

void Generator::GenerateRooms()
{
	const float minRoomSize = m_input -> m_minRoomSize;
	const float diffRoomSize = m_input -> m_maxRoomSize - m_input -> m_minRoomSize;

	m_rooms.reserve(m_totalRoomCount);
	bt::Node<Cell>::defaultTraversal = bt::Traversal::POSTORDER;

	for (auto& btNode : *m_root)
	{
		if ((btNode.m_flags & (1 << Cell::Flag::GENERATE_ROOMS)) == 0)
			continue;

		const float a = m_random.GetFP32() * diffRoomSize + minRoomSize;
		const float b = m_random.GetFP32() * diffRoomSize + minRoomSize;

		Vec priSize(static_cast<int>(btNode.m_space.w * a), static_cast<int>(btNode.m_space.h * b));

		if (priSize.x < roomSizeLimit)
			priSize.x = roomSizeLimit;

		if (priSize.y < roomSizeLimit)
			priSize.y = roomSizeLimit;

		Vec priPos(btNode.m_space.x, btNode.m_space.y);
		Vec remSize(btNode.m_space.w - priSize.x, btNode.m_space.h - priSize.y);

		Vec secPos(-1, 0);
		Vec secSize(0, 0);

		if (m_random.GetFP32() < m_input -> m_doubleRoomProb)
		{
			int Vec::*incAxis; int Vec::*decAxis;

			if (remSize.x > remSize.y) { incAxis = &Vec::x; decAxis = &Vec::y; }
			else { incAxis = &Vec::y; decAxis = &Vec::x; }

			secSize.*decAxis = priSize.*decAxis >> 1;
			if (secSize.*decAxis < roomSizeLimit)
				goto skip_double_room;

			const int extra = static_cast<int>(remSize.*incAxis * (m_random.GetFP32() * diffRoomSize + minRoomSize));
			if (extra <= 0)
				goto skip_double_room;

			secSize.*incAxis = priSize.*incAxis + extra;
			remSize.*incAxis -= extra;
			secPos = priPos;

			auto [c, d] = m_random.Get32P();

			if (c %= 3; c < 2)
				priPos.*incAxis += extra >> c;

			if (d %= 3; d < 2)
				secPos.*decAxis += (priSize.*decAxis - secSize.*decAxis) >> d;
		}

		skip_double_room:
		const auto [c, d] = m_random.Get32P();
		const auto [e, f] = m_random.Get32P();

		const Vec offset(c % (remSize.x + 1), d % (remSize.y + 1));

		Room& room = m_rooms.emplace_back();
		room.m_rects.emplace_back(priPos.x + offset.x, priPos.y + offset.y, priSize.x, priSize.y);

		if (secPos.x == -1)
		{
			const Rect& rect = room.m_rects.front();

			room.m_pos.x = rect.x + 1 + (e % (rect.w - 2));
			room.m_pos.y = rect.y + 1 + (f % (rect.h - 2));
		}
		else
		{
			room.m_rects.emplace_back(secPos.x + offset.x, secPos.y + offset.y, secSize.x, secSize.y);
			m_totalRoomCount++;

			const bool randBool = m_random.GetBit();
			const Rect& priRect = room.m_rects.at(static_cast<size_t>(randBool));
			const Rect& secRect = room.m_rects.at(static_cast<size_t>(!randBool));

			room.m_pos.x = priRect.x + 1;
			room.m_pos.y = priRect.y + 1;

			if (priRect.w > secRect.w)
			{
				const int flag1 = static_cast<int>(secRect.x > priRect.x);
				const int flag2 = static_cast<int>(secRect.x + secRect.w < priRect.x + priRect.w);

				room.m_pos.x += e % (priRect.w - 2 - flag1 - flag2);
				room.m_pos.y += f % (priRect.h - 2);

				if (room.m_pos.x >= secRect.x)
				{
					room.m_pos.x += flag1;
					room.m_pos.x += static_cast<int>(room.m_pos.x >= secRect.x + secRect.w - 1);
				}
			}
			else
			{
				const int flag1 = static_cast<int>(secRect.y > priRect.y);
				const int flag2 = static_cast<int>(secRect.y + secRect.h < priRect.y + priRect.h);

				room.m_pos.x += e % (priRect.w - 2);
				room.m_pos.y += f % (priRect.h - 2 - flag1 - flag2);

				if (room.m_pos.y >= secRect.y)
				{
					room.m_pos.y += flag1;
					room.m_pos.y += static_cast<int>(room.m_pos.y >= secRect.y + secRect.h - 1);
				}
			}
		}

		room.ComputeEdges();
		CreateRoomNodes(btNode.m_space, room);
	}
}

void Generator::GenerateOutput()
{
	int ne = 0; int sw = 0;
	for (const Room& room : m_rooms)
	{
		ne += ((room.m_path >> Dir::NORTH) & 1) + ((room.m_path >> Dir::EAST) & 1);
		sw += ((room.m_path >> Dir::SOUTH) & 1) + ((room.m_path >> Dir::WEST) & 1);
	}

	m_output -> m_rooms.reserve(static_cast<size_t>(m_totalRoomCount));
	m_output -> m_paths.reserve(static_cast<size_t>(ne + m_partialPathCount));
	m_output -> m_entrances.reserve(static_cast<size_t>(ne + sw));

	for (Room& room : m_rooms)
	{
		for (const Rect& rect : room.m_rects)
			m_output -> m_rooms.push_back(rect);

		if (room.m_path & (1 << Dir::NORTH))
		{
			const int edg = room.m_edges[Dir::NORTH];
			const Point ext = room.m_links[Dir::NORTH] -> m_pos;

			m_output -> m_entrances.emplace_back(ext.x, edg);
			m_output -> m_paths.emplace_back(ext, Vec(0, edg - ext.y));
		}

		if (room.m_path & (1 << Dir::EAST))
		{
			const int edg = room.m_edges[Dir::EAST];
			const Point ext = room.m_links[Dir::EAST] -> m_pos;

			m_output -> m_entrances.emplace_back(edg, ext.y);
			m_output -> m_paths.emplace_back(ext, Vec(edg - ext.x, 0));
		}

		if (room.m_path & (1 << Dir::SOUTH))
		{
			const int edg = room.m_edges[Dir::SOUTH];
			const Point ext = room.m_links[Dir::SOUTH] -> m_pos;

			m_output -> m_entrances.emplace_back(ext.x, edg);
			m_output -> m_paths.emplace_back(ext, Vec(0, edg - ext.y));

			room.m_links[Dir::SOUTH] -> m_path &= ~(1 << Dir::NORTH);
		}

		if (room.m_path & (1 << Dir::WEST))
		{
			const int edg = room.m_edges[Dir::WEST];
			const Point ext = room.m_links[Dir::WEST] -> m_pos;

			m_output -> m_entrances.emplace_back(edg, ext.y);
			m_output -> m_paths.emplace_back(ext, Vec(edg - ext.x, 0));

			room.m_links[Dir::WEST] -> m_path &= ~(1 << Dir::EAST);
		}
	}

	for (const auto& [pos, node] : m_nodes)
	{
		const auto& [xCrr, yCrr] = pos;
		if (node.m_path & (1 << Dir::NORTH))
		{
			const auto [xAdj, yAdj] = node.m_links[Dir::NORTH] -> m_pos;
			m_output -> m_paths.emplace_back(Point(xCrr, yCrr), Vec(xAdj - xCrr, yAdj - yCrr));
		}

		if (node.m_path & (1 << Dir::EAST))
		{
			const auto [xAdj, yAdj] = node.m_links[Dir::EAST] -> m_pos;
			m_output -> m_paths.emplace_back(Point(xCrr, yCrr), Vec(xAdj - xCrr, yAdj - yCrr));
		}
	}
}

uint32_t Generator::GenerateTree(bt::Node<Cell>& btNode, int left)
{
	if (left <= m_input -> m_sparseAreaDepth)
		btNode.m_flags |= static_cast<uint32_t>(m_random.GetFP32() < m_input -> m_sparseAreaProb) << Cell::Flag::SPARSE_AREA;

	if (left == m_deltaDepth)
	{
		if (m_deltaDepth > 0) m_targetDepth = m_random.Get32() % (m_deltaDepth + 1);
		else goto no_more;
	}

	if (left <= m_targetDepth)
	{
		no_more:
		Rect& space = btNode.m_space;

		space.x += m_spaceOffset; space.y += m_spaceOffset;
		space.w -= m_spaceShrink; space.h -= m_spaceShrink;

		CreateSpaceNodes(space);
		if (btNode.m_flags & (1 << Cell::Flag::SPARSE_AREA))
		{
			if (m_random.GetFP32() >= m_input -> m_sparseAreaDens)
				return 0;
		}

		btNode.m_flags |= 1 << Cell::Flag::GENERATE_ROOMS;
		btNode.m_roomOffset = m_totalRoomCount++;
		btNode.m_roomCount = 1;

		return 1 << Cell::Flag::CONNECT_ROOMS;
	}

	int Rect::*xy; int Rect::*wh;
	Rect& crrSpace = btNode.m_space;

	if (crrSpace.w < crrSpace.h) { xy = &Rect::y; wh = &Rect::h; }
	else { xy = &Rect::x; wh = &Rect::w; }

	const float c = m_random.GetFP32() * (m_input -> m_spaceSizeRandomness) + m_minSpaceRand;

	const int totalSize = crrSpace.*wh;
	const int randSize = static_cast<int>(totalSize * c);

	if (randSize < m_minSpaceSize || totalSize - randSize < m_minSpaceSize)
		goto no_more;

	btNode.m_left = static_cast<bt::Node<Cell>*>(operator new[](sizeof(bt::Node<Cell>) * 2));
	btNode.m_right = btNode.m_left + 1;

	new(btNode.m_left) bt::Node<Cell>(&btNode, btNode);
	new(btNode.m_right) bt::Node<Cell>(&btNode, btNode);

	btNode.m_left -> m_space.*wh = randSize;
	btNode.m_right -> m_space.*xy += randSize;
	btNode.m_right -> m_space.*wh -= randSize;

	const uint32_t l = GenerateTree(*btNode.m_left, --left);
	const uint32_t r = GenerateTree(*btNode.m_right, left);

	btNode.m_roomOffset = std::min(btNode.m_left -> m_roomOffset, btNode.m_right -> m_roomOffset);
	btNode.m_roomCount = btNode.m_left -> m_roomCount + btNode.m_right -> m_roomCount;
	btNode.m_flags |= l & r;

	return l | r;
}

void Generator::DeleteTree(bt::Node<Cell>* btNode)
{
	if (btNode -> m_left == nullptr)
		return;

	DeleteTree(btNode -> m_right);
	btNode -> m_right -> ~Node<Cell>();

	DeleteTree(btNode -> m_left);
	btNode -> m_left -> ~Node<Cell>();

	operator delete[](btNode -> m_left);
}

Node& Generator::RegisterNode(int x, int y)
{
	return m_nodes.emplace(std::make_pair(x, y), Node(x, y)).first -> second;
}

void Generator::CreateSpaceNodes(Rect& space)
{
	const int d1 = m_spaceOffset - 1;

	const int xMin = space.x - m_spaceOffset;
	const int yMin = space.y - m_spaceOffset;
	const int xMax = space.x + space.w + d1;
	const int yMax = space.y + space.h + d1;

	RegisterNode(xMax, yMax).m_path |= (1 << Dir::NORTH) | (1 << Dir::WEST);
	RegisterNode(xMin, yMax).m_path |= 1 << Dir::NORTH;
	RegisterNode(xMax, yMin).m_path |= 1 << Dir::WEST;
	RegisterNode(xMin, yMin);
}

void Generator::CreateRoomNodes(Rect& space, Room& room)
{
	const auto& [xS, yS, wS, hS] = space;
	const auto& [xR, yR] = room.m_pos;

	const int d0 = m_spaceOffset;
	const int d1 = m_spaceOffset - 1;

	auto& [north, east, south, west] = room.m_links;

	north = &RegisterNode(xR, yS - d0);
	north -> m_path |= 1 << Dir::WEST;
	north -> m_links[Dir::SOUTH] = &room;

	east = &RegisterNode(xS + wS + d1, yR);
	east -> m_path |= 1 << Dir::NORTH;
	east -> m_links[Dir::WEST] = &room;

	south = &RegisterNode(xR, yS + hS + d1);
	south -> m_path |= 1 << Dir::WEST;
	south -> m_links[Dir::NORTH] = &room;

	west = &RegisterNode(xS - d0, yR);
	west -> m_path |= 1 << Dir::NORTH;
	west -> m_links[Dir::EAST] = &room;
}

void Generator::Generate(const GenInput* input, GenOutput* output)
{
	m_input = input;
	m_output = output;

	Clear();
	Prepare();
	GenerateTree(*m_root, m_input -> m_maxDepth);
	GenerateRooms();
	LinkNodes();
	FindPaths();
	OptimizeNodes();
	GenerateOutput();
}