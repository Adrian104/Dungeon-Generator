// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "dgen_impl.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace dg::impl
{
	Vertex Vertex::s_sentinel(std::numeric_limits<decltype(Vertex::m_status)>::max());

	void Vertex::Unlink()
	{
		for (int i = 0; i < 4; i++)
		{
			m_links[i]->m_links[i ^ 0b10] = &Vertex::s_sentinel;
			m_links[i] = &Vertex::s_sentinel;
		}
	}

	Tag::Tag(int high, int low)
		: m_pos((static_cast<uint64_t>(high) << 32) | static_cast<uint64_t>(low)) {}

	Tag::Tag(int high, int low, uint8_t linkBits, uint8_t origin, uint64_t index)
		: Tag(high, low)
	{
		m_data.m_index = index;
		m_data.m_origin = origin;
		m_data.m_linkBits = linkBits;
		m_data.m_hasIndex = 1;
	}

	RadixSort::RadixSort(const size_t maxSize)
		: m_memory(operator new[](sizeof(size_t) * 256 + sizeof(Tag) * maxSize)) {}

	RadixSort::~RadixSort() { operator delete[](m_memory); }

	void RadixSort::Sort(Tag* arr, const size_t size) const
	{
		static constexpr auto s_overflow = static_cast<size_t>(0) - 1;
		size_t* const count = static_cast<size_t*>(m_memory);
		Tag* temp = reinterpret_cast<Tag*>(count + 256);

		for (int bits = 0; bits < 64; bits += 8)
		{
			for (size_t i = 0; i < 256; i++)
				count[i] = 0;

			for (size_t i = 0; i < size; i++)
				++count[(arr[i].m_pos >> bits) & 0xFF];

			size_t prev = *count;
			for (size_t i = 1; i < 256; i++)
				prev = (count[i] += prev);

			for (size_t i = size - 1; i != s_overflow; i--)
				temp[--count[(arr[i].m_pos >> bits) & 0xFF]] = arr[i];

			std::swap(arr, temp);
		}
	}

	void Generator::Clear()
	{
		m_tags.clear();
		m_rooms.clear();
		m_vertices.clear();
		m_vertexHeap.Clear();

		if (m_rootNode != nullptr)
		{
			DeleteTree(m_rootNode);

			delete m_rootNode;
			m_rootNode = nullptr;
		}
	}

	void Generator::Prepare()
	{
		if (m_input->m_maxRoomSize <= 0)
			throw std::runtime_error("Variable 'maxRoomSize' is not a positive number");

		*m_output = {};
		m_random.Seed(m_input->m_seed);

		m_spaceOffset = m_input->m_spaceInterdistance + 1;
		m_spaceShrink = (m_spaceOffset << 1) - 1;

		m_minSpaceSize = static_cast<int>(s_roomSizeLimit / m_input->m_maxRoomSize) + m_spaceShrink;
		m_minSpaceRand = (1.0f - m_input->m_spaceSizeRandomness) * 0.5f;

		if (m_input->m_width <= m_minSpaceSize || m_input->m_height <= m_minSpaceSize)
			throw std::runtime_error("Root node is too small");

		m_rootNode = new Node<Cell>(nullptr, Cell(m_input->m_width - 1, m_input->m_height - 1));

		m_targetDepth = 0;
		m_statusCounter = 1;
		m_totalRoomCount = 0;
		m_partialPathCount = 0;

		m_deltaDepth = m_input->m_maxDepth - m_input->m_minDepth;
		m_randPathDepth = m_input->m_maxDepth - m_input->m_extraPathDepth;
	}

	uint32_t Generator::GenerateTree(Node<Cell>& node, int left)
	{
		node.m_flags |= static_cast<uint32_t>(left <= m_randPathDepth) << Cell::Flag::RANDOM_PATH;

		if (left <= m_input->m_sparseAreaDepth)
			node.m_flags |= static_cast<uint32_t>(m_random.GetFP32() < m_input->m_sparseAreaProb) << Cell::Flag::SPARSE_AREA;

		if (left == m_deltaDepth && m_deltaDepth > 0)
			m_targetDepth = m_random.Get32() % (m_deltaDepth + 1);

		if (left <= m_targetDepth)
			return MakeLeafCell(node);

		static constexpr std::pair<int Rect::*, int Rect::*> xw = std::make_pair(&Rect::x, &Rect::w);
		static constexpr std::pair<int Rect::*, int Rect::*> yh = std::make_pair(&Rect::y, &Rect::h);

		const auto& [xy, wh] = node.m_space.w >= node.m_space.h ? xw : yh;
		const float c = m_random.GetFP32() * (m_input->m_spaceSizeRandomness) + m_minSpaceRand;

		const int totalSize = node.m_space.*wh;
		const int randSize = static_cast<int>(totalSize * c);

		if (randSize < m_minSpaceSize || totalSize - randSize < m_minSpaceSize)
			return MakeLeafCell(node);

		node.m_left = static_cast<Node<Cell>*>(operator new[](sizeof(Node<Cell>) * 2));
		node.m_right = node.m_left + 1;

		new(node.m_left) Node<Cell>(&node, node);
		new(node.m_right) Node<Cell>(&node, node);

		node.m_left->m_space.*wh = randSize;
		node.m_right->m_space.*xy += randSize;
		node.m_right->m_space.*wh -= randSize;

		const uint32_t l = GenerateTree(*node.m_left, --left);
		const uint32_t r = GenerateTree(*node.m_right, left);

		node.m_roomOffset = std::min(node.m_left->m_roomOffset, node.m_right->m_roomOffset);
		node.m_roomCount = node.m_left->m_roomCount + node.m_right->m_roomCount;
		node.m_flags |= l & r;

		return l | r;
	}

	uint32_t Generator::MakeLeafCell(Node<Cell>& node)
	{
		Rect& space = node.m_space;

		space.x += m_spaceOffset; space.y += m_spaceOffset;
		space.w -= m_spaceShrink; space.h -= m_spaceShrink;

		const int d1 = m_spaceOffset - 1;

		const int xMin = space.x - m_spaceOffset;
		const int yMin = space.y - m_spaceOffset;
		const int xMax = space.x + space.w + d1;
		const int yMax = space.y + space.h + d1;

		m_tags.emplace_back(xMax, yMax).m_data.m_linkBits = (1ULL << Dir::NORTH) | (1ULL << Dir::WEST);
		m_tags.emplace_back(xMin, yMax).m_data.m_linkBits = 1ULL << Dir::NORTH;
		m_tags.emplace_back(xMax, yMin).m_data.m_linkBits = 1ULL << Dir::WEST;
		m_tags.emplace_back(xMin, yMin);

		uint32_t flagsToReturn = 0;
		if ((node.m_flags & (1 << Cell::Flag::SPARSE_AREA)) == 0 || m_random.GetFP32() < m_input->m_sparseAreaDens)
		{
			flagsToReturn = 1 << Cell::Flag::CONNECT_ROOMS;
			node.m_flags |= 1 << Cell::Flag::GENERATE_ROOMS;
			node.m_roomOffset = m_totalRoomCount++;
			node.m_roomCount = 1;
		}

		return flagsToReturn;
	}

	void Generator::GenerateRooms()
	{
		const float minRoomSize = m_input->m_minRoomSize;
		const float diffRoomSize = m_input->m_maxRoomSize - m_input->m_minRoomSize;

		m_rooms.reserve(static_cast<size_t>(m_totalRoomCount));
		m_output->m_rooms.reserve(static_cast<size_t>(m_totalRoomCount) << 1);

		for (auto& node : m_rootNode->Postorder())
		{
			if ((node.m_flags & (1 << Cell::Flag::GENERATE_ROOMS)) == 0)
				continue;

			const float a = m_random.GetFP32() * diffRoomSize + minRoomSize;
			const float b = m_random.GetFP32() * diffRoomSize + minRoomSize;

			Vec priSize(static_cast<int>(node.m_space.w * a), static_cast<int>(node.m_space.h * b));

			if (priSize.x < s_roomSizeLimit)
				priSize.x = s_roomSizeLimit;

			if (priSize.y < s_roomSizeLimit)
				priSize.y = s_roomSizeLimit;

			Vec priPos(node.m_space.x, node.m_space.y);
			Vec remSize(node.m_space.w - priSize.x, node.m_space.h - priSize.y);

			Vec secPos(-1, 0);
			Vec secSize(0, 0);

			if (m_random.GetFP32() < m_input->m_doubleRoomProb)
			{
				static constexpr std::pair<int Vec::*, int Vec::*> xy = std::make_pair(&Vec::x, &Vec::y);
				static constexpr std::pair<int Vec::*, int Vec::*> yx = std::make_pair(&Vec::y, &Vec::x);

				const auto& [incAxis, decAxis] = remSize.x > remSize.y ? xy : yx;
				secSize.*decAxis = priSize.*decAxis >> 1;

				if (secSize.*decAxis >= s_roomSizeLimit)
				{
					const int extra = static_cast<int>(remSize.*incAxis * (m_random.GetFP32() * diffRoomSize + minRoomSize));
					if (extra > 0)
					{
						secSize.*incAxis = priSize.*incAxis + extra;
						remSize.*incAxis -= extra;
						secPos = priPos;

						auto [c, d] = m_random.Get32P();

						if (c %= 3; c < 2)
							priPos.*incAxis += extra >> c;

						if (d %= 3; d < 2)
							secPos.*decAxis += (priSize.*decAxis - secSize.*decAxis) >> d;
					}
				}
			}

			const auto [c, d] = m_random.Get32P();
			const Vec offset(c % (remSize.x + 1), d % (remSize.y + 1));

			Point pos[2]{};
			Room& room = m_rooms.emplace_back(node);

			room.m_rectBegin = m_output->m_rooms.size();
			m_output->m_rooms.emplace_back(priPos.x + offset.x, priPos.y + offset.y, priSize.x, priSize.y);

			room.m_rectEnd = m_output->m_rooms.size();
			room.m_pos = Point(priPos.x + offset.x + (priSize.x >> 1), priPos.y + offset.y + (priSize.y >> 1));

			if (secPos.x == -1)
			{
				const Rect& rect = m_output->m_rooms[room.m_rectBegin];

				const auto [e, f] = m_random.Get32P();
				const auto [g, h] = m_random.Get32P();

				pos[0].x = rect.x + 1 + (e % (rect.w - 2));
				pos[0].y = rect.y + 1 + (f % (rect.h - 2));
				pos[1].x = rect.x + 1 + (g % (rect.w - 2));
				pos[1].y = rect.y + 1 + (h % (rect.h - 2));
			}
			else
			{
				m_output->m_rooms.emplace_back(secPos.x + offset.x, secPos.y + offset.y, secSize.x, secSize.y);
				room.m_rectEnd = m_output->m_rooms.size();

				const bool randBool = m_random.GetBit();
				const Rect& priRect = m_output->m_rooms[room.m_rectBegin + static_cast<size_t>(randBool)];
				const Rect& secRect = m_output->m_rooms[room.m_rectBegin + static_cast<size_t>(!randBool)];

				auto CalculatePos = [this, &priRect, &secRect](Point& pos) -> void
				{
					const auto [e, f] = m_random.Get32P();

					pos.x = priRect.x + 1;
					pos.y = priRect.y + 1;

					if (priRect.w > secRect.w)
					{
						const int flag1 = static_cast<int>(secRect.x > priRect.x);
						const int flag2 = static_cast<int>(secRect.x + secRect.w < priRect.x + priRect.w);

						pos.x += e % (priRect.w - 2 - flag1 - flag2);
						pos.y += f % (priRect.h - 2);

						if (pos.x >= secRect.x)
						{
							pos.x += flag1;
							pos.x += static_cast<int>(pos.x >= secRect.x + secRect.w - 1);
						}
					}
					else
					{
						const int flag1 = static_cast<int>(secRect.y > priRect.y);
						const int flag2 = static_cast<int>(secRect.y + secRect.h < priRect.y + priRect.h);

						pos.x += e % (priRect.w - 2);
						pos.y += f % (priRect.h - 2 - flag1 - flag2);

						if (pos.y >= secRect.y)
						{
							pos.y += flag1;
							pos.y += static_cast<int>(pos.y >= secRect.y + secRect.h - 1);
						}
					}
				};

				CalculatePos(pos[0]);
				CalculatePos(pos[1]);
			}

			auto& ent = room.m_entrances;

			ent[Dir::NORTH] = Point(pos[0].x, std::numeric_limits<int>::max());
			ent[Dir::EAST] = Point(0, pos[0].y);
			ent[Dir::SOUTH] = Point(pos[1].x, 0);
			ent[Dir::WEST] = Point(std::numeric_limits<int>::max(), pos[1].y);

			for (size_t i = room.m_rectBegin; i < room.m_rectEnd; i++)
			{
				const Rect& rect = m_output->m_rooms[i];
				const int xPlusW = rect.x + rect.w;
				const int yPlusH = rect.y + rect.h;

				if (pos[0].x >= rect.x && pos[0].x < xPlusW && ent[Dir::NORTH].y > rect.y)
					ent[Dir::NORTH].y = rect.y;

				if (pos[0].y >= rect.y && pos[0].y < yPlusH && ent[Dir::EAST].x < xPlusW)
					ent[Dir::EAST].x = xPlusW;

				if (pos[1].x >= rect.x && pos[1].x < xPlusW && ent[Dir::SOUTH].y < yPlusH)
					ent[Dir::SOUTH].y = yPlusH;

				if (pos[1].y >= rect.y && pos[1].y < yPlusH && ent[Dir::WEST].x > rect.x)
					ent[Dir::WEST].x = rect.x;
			}

			room.m_entrances[Dir::EAST].x--;
			room.m_entrances[Dir::SOUTH].y--;

			const int d0 = m_spaceOffset;
			const int d1 = m_spaceOffset - 1;

			const auto& [xS, yS, wS, hS] = node.m_space;
			const uint64_t index = static_cast<uint64_t>(node.m_roomOffset);

			m_tags.emplace_back(pos[0].x, yS - d0, static_cast<uint8_t>(1 << Dir::WEST), static_cast<uint8_t>(Dir::SOUTH), index);
			m_tags.emplace_back(xS + wS + d1, pos[0].y, static_cast<uint8_t>(1 << Dir::NORTH), static_cast<uint8_t>(Dir::WEST), index);
			m_tags.emplace_back(pos[1].x, yS + hS + d1, static_cast<uint8_t>(1 << Dir::WEST), static_cast<uint8_t>(Dir::NORTH), index);
			m_tags.emplace_back(xS - d0, pos[1].y, static_cast<uint8_t>(1 << Dir::NORTH), static_cast<uint8_t>(Dir::EAST), index);
		}
	}

	void Generator::CreateVertices()
	{
		RadixSort rs(m_tags.size());
		rs.Sort(m_tags.data(), m_tags.size());

		size_t count = 0;
		uint64_t pos = std::numeric_limits<uint64_t>::max();

		for (const Tag& tag : m_tags)
		{
			count += pos != tag.m_pos;
			pos = tag.m_pos;
		}

		std::vector<Tag> revTags(count);
		m_vertices.resize(count);

		Tag* revTag = revTags.data() - 1;
		Vertex* vertex = m_vertices.data() - 1;

		Vertex* pri[2] = { &Vertex::s_sentinel, &Vertex::s_sentinel };
		Vertex* sec[2] = { &Vertex::s_sentinel, &Vertex::s_sentinel };

		pos = std::numeric_limits<uint64_t>::max();
		for (const Tag& tag : m_tags)
		{
			const size_t diff = pos != tag.m_pos;

			pos = tag.m_pos;
			vertex += diff;
			revTag += diff;

			const uint64_t xPos = pos >> 32;

			revTag->m_pos = (pos << 32) | xPos;
			revTag->m_vertex = vertex;

			vertex->m_pos.x = static_cast<int>(xPos);
			vertex->m_pos.y = static_cast<int>(pos & 0xFFFFFFFF);
			vertex->m_path |= tag.m_data.m_linkBits;

			pri[1] = vertex;
			sec[1] = m_rooms.data() + tag.m_data.m_index;

			const int exists = tag.m_data.m_hasIndex;

			pri[exists]->m_links[tag.m_data.m_origin] = sec[exists];
			sec[exists]->m_links[tag.m_data.m_origin ^ 0b10] = pri[exists];
		}

		m_tags.clear();
		for (Vertex& crr : m_vertices)
		{
			const int exists = (crr.m_path >> Dir::NORTH) & 1;

			pri[1] = &crr;
			pri[exists]->m_links[Dir::NORTH] = sec[exists];
			sec[exists]->m_links[Dir::SOUTH] = pri[exists];
			sec[1] = pri[1];
		}

		rs.Sort(revTags.data(), revTags.size());
		for (const Tag& tag : revTags)
		{
			Vertex* const crr = tag.m_vertex;
			const int exists = (crr->m_path >> Dir::WEST) & 1;

			pri[1] = crr;
			pri[exists]->m_links[Dir::WEST] = sec[exists];
			sec[exists]->m_links[Dir::EAST] = pri[exists];
			sec[1] = pri[1];

			crr->m_path = 0;
		}
	}

	void Generator::FindPaths()
	{
		for (auto& node : m_rootNode->Postorder())
		{
			if ((node.m_flags & (1 << Cell::Flag::CONNECT_ROOMS)) == 0)
				continue;

			if (node.m_flags & (1 << Cell::Flag::RANDOM_PATH))
			{
				int leftIndex = node.m_left->m_roomOffset;
				const int leftCount = node.m_left->m_roomCount;

				int rightIndex = node.m_right->m_roomOffset;
				const int rightCount = node.m_right->m_roomCount;

				if (leftCount > 1)
					leftIndex += m_random.Get32() % leftCount;

				if (rightCount > 1)
					rightIndex += m_random.Get32() % rightCount;

				FindPath(m_rooms.data() + leftIndex, m_rooms.data() + rightIndex);
			}
			else
			{
				int n = m_input->m_extraPathCount + 1;
				const int d = m_input->m_extraPathCount + 2;

				const auto [xL, yL, wL, hL] = node.m_left->m_space;
				const auto [xR, yR, wR, hR] = node.m_right->m_space;

				do
				{
					const Point center = xL + wL <= xR ? Point(xR, yR + n * hR / d) : Point(xR + n * wR / d, yR);
					const int leftIndex = GetNearestRoomTo(center, node.m_left);
					const int rightIndex = GetNearestRoomTo(center, node.m_right);

					FindPath(m_rooms.data() + leftIndex, m_rooms.data() + rightIndex);

				} while (--n > 0);
			}
		}
	}

	void Generator::FindPath(Room* const start, Room* const stop)
	{
		const float factors[2] = { 1.0f, m_input->m_pathCostFactor };
		Vertex* vertex = start;
		start->m_gcost = 0;

		do
		{
			vertex->m_status = m_statusCounter + 1;
			Room* const room = vertex->ToRoom();

			for (uint8_t i = 0; i < 4; i++)
			{
				Vertex* const adjacent = vertex->m_links[i];
				if (adjacent->m_status > m_statusCounter)
					continue;

				Point p1, p2;
				if (room == nullptr)
				{
					Room* const adjRoom = adjacent->ToRoom();

					p1 = vertex->m_pos;
					p2 = (adjRoom != nullptr) ? adjRoom->m_entrances[i ^ 0b10] : adjacent->m_pos;
				}
				else
				{
					p1 = adjacent->m_pos;
					p2 = room->m_entrances[i];
				}

				const float diff = static_cast<float>(std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y));
				const float newGCost = vertex->m_gcost + diff * factors[(vertex->m_path >> i) & 1];

				if (adjacent->m_status < m_statusCounter)
				{
					const int dx = stop->m_pos.x - adjacent->m_pos.x;
					const int dy = stop->m_pos.y - adjacent->m_pos.y;

					const float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));

					adjacent->m_hcost = dist * m_input->m_heuristicFactor;
					adjacent->m_status = m_statusCounter;
					adjacent->m_origin = i;
					adjacent->m_gcost = newGCost;

					m_vertexHeap.Push(newGCost + adjacent->m_hcost, adjacent);
				}
				else if (newGCost < adjacent->m_gcost)
				{
					adjacent->m_origin = i;
					adjacent->m_gcost = newGCost;

					m_vertexHeap.Push(newGCost + adjacent->m_hcost, adjacent);
				}
			}

			do
			{
				vertex = m_vertexHeap.TopObject();
				m_vertexHeap.Pop();

			} while (vertex->m_status > m_statusCounter);

		} while (vertex != stop);

		m_statusCounter += 2;
		m_vertexHeap.Clear();

		do
		{
			const uint8_t origin = vertex->m_origin;
			const uint8_t realOrigin = origin ^ 0b10;

			vertex->m_path |= 1 << realOrigin;
			vertex = vertex->m_links[realOrigin];
			vertex->m_path |= 1 << origin;

		} while (vertex != start);
	}

	void Generator::OptimizeVertices()
	{
		const uint8_t maskEW = m_input->m_generateFewerPaths ? 0b1010 : 0b1111;
		const uint8_t maskNS = m_input->m_generateFewerPaths ? 0b0101 : 0b1111;

		for (Vertex& vertex : m_vertices)
		{
			uint8_t& path = vertex.m_path;
			Vertex** links = vertex.m_links;

			if (path == 0)
			{
				vertex.Unlink();
				continue;
			}

			if ((path & maskEW) == 0b1010)
			{
				Vertex* const east = links[Dir::EAST];
				Vertex* const west = links[Dir::WEST];

				if (east->ToRoom() == nullptr || west->ToRoom() == nullptr)
				{
					east->m_links[Dir::WEST] = west;
					west->m_links[Dir::EAST] = east;

					links[Dir::EAST] = &Vertex::s_sentinel;
					links[Dir::WEST] = &Vertex::s_sentinel;

					if (path &= ~0b1010; path == 0)
					{
						vertex.Unlink();
						continue;
					}
				}
			}

			if ((path & maskNS) == 0b0101)
			{
				Vertex* const north = links[Dir::NORTH];
				Vertex* const south = links[Dir::SOUTH];

				if (north->ToRoom() == nullptr || south->ToRoom() == nullptr)
				{
					north->m_links[Dir::SOUTH] = south;
					south->m_links[Dir::NORTH] = north;

					links[Dir::NORTH] = &Vertex::s_sentinel;
					links[Dir::SOUTH] = &Vertex::s_sentinel;

					if (path &= ~0b0101; path == 0)
					{
						vertex.Unlink();
						continue;
					}
				}
			}

			m_partialPathCount += ((path >> Dir::NORTH) & 1) + ((path >> Dir::EAST) & 1);
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

		m_output->m_rooms.shrink_to_fit();
		m_output->m_paths.reserve(static_cast<size_t>(ne) + static_cast<size_t>(m_partialPathCount));
		m_output->m_entrances.reserve(static_cast<size_t>(ne) + static_cast<size_t>(sw));

		for (Room& room : m_rooms)
		{
			if (room.m_path & (1 << Dir::NORTH))
			{
				const Point ent = room.m_entrances[Dir::NORTH];
				const Point ext = room.m_links[Dir::NORTH]->m_pos;

				m_output->m_entrances.emplace_back(ent);
				m_output->m_paths.emplace_back(ext, Vec(0, ent.y - ext.y));
			}

			if (room.m_path & (1 << Dir::EAST))
			{
				const Point ent = room.m_entrances[Dir::EAST];
				const Point ext = room.m_links[Dir::EAST]->m_pos;

				m_output->m_entrances.emplace_back(ent);
				m_output->m_paths.emplace_back(ext, Vec(ent.x - ext.x, 0));
			}

			if (room.m_path & (1 << Dir::SOUTH))
			{
				const Point ent = room.m_entrances[Dir::SOUTH];
				const Point ext = room.m_links[Dir::SOUTH]->m_pos;

				m_output->m_entrances.emplace_back(ent);
				m_output->m_paths.emplace_back(ext, Vec(0, ent.y - ext.y));

				room.m_links[Dir::SOUTH]->m_path &= ~(1 << Dir::NORTH);
			}

			if (room.m_path & (1 << Dir::WEST))
			{
				const Point ent = room.m_entrances[Dir::WEST];
				const Point ext = room.m_links[Dir::WEST]->m_pos;

				m_output->m_entrances.emplace_back(ent);
				m_output->m_paths.emplace_back(ext, Vec(ent.x - ext.x, 0));

				room.m_links[Dir::WEST]->m_path &= ~(1 << Dir::EAST);
			}
		}

		for (const Vertex& vertex : m_vertices)
		{
			const auto& [xCrr, yCrr] = vertex.m_pos;
			if (vertex.m_path & (1 << Dir::NORTH))
			{
				const auto [xAdj, yAdj] = vertex.m_links[Dir::NORTH]->m_pos;
				m_output->m_paths.emplace_back(Point(xCrr, yCrr), Vec(xAdj - xCrr, yAdj - yCrr));
			}

			if (vertex.m_path & (1 << Dir::EAST))
			{
				const auto [xAdj, yAdj] = vertex.m_links[Dir::EAST]->m_pos;
				m_output->m_paths.emplace_back(Point(xCrr, yCrr), Vec(xAdj - xCrr, yAdj - yCrr));
			}
		}
	}

	void Generator::DeleteTree(Node<Cell>* node)
	{
		if (node->m_left == nullptr)
			return;

		DeleteTree(node->m_right);
		node->m_right->~Node<Cell>();

		DeleteTree(node->m_left);
		node->m_left->~Node<Cell>();

		operator delete[](node->m_left);
	}

	int Generator::GetNearestRoomTo(const Point point, Node<Cell>* node)
	{
		if (node->m_left == nullptr)
			return node->m_roomOffset;

		const auto [xL, yL, wL, hL] = node->m_left->m_space;
		const auto [xR, yR, wR, hR] = node->m_right->m_space;

		const int l = std::abs(xL + (wL >> 1) - point.x) + std::abs(yL + (hL >> 1) - point.y);
		const int r = std::abs(xR + (wR >> 1) - point.x) + std::abs(yR + (hR >> 1) - point.y);

		static constexpr int s_maxOffset = std::numeric_limits<int>::max();
		Node<Cell>* const nextNodes[2] = { node->m_left, node->m_right };

		const int offset = GetNearestRoomTo(point, nextNodes[l > r]);
		return offset != s_maxOffset ? offset : GetNearestRoomTo(point, nextNodes[l <= r]);
	}

	void Generator::Generate(const Input* input, Output* output)
	{
		m_input = input;
		m_output = output;

		Clear();
		Prepare();
		GenerateTree(*m_rootNode, m_input->m_maxDepth);
		GenerateRooms();
		CreateVertices();
		FindPaths();
		OptimizeVertices();
		GenerateOutput();
	}
}

namespace dg
{
	Input GetExampleInput()
	{
		Input in{};

		in.m_seed = 0;
		in.m_width = 800;
		in.m_height = 800;
		in.m_minDepth = 7;
		in.m_maxDepth = 8;
		in.m_spaceInterdistance = 1;
		in.m_spaceSizeRandomness = 0.35f;
		in.m_sparseAreaDepth = 1;
		in.m_sparseAreaDens = 0.4f;
		in.m_sparseAreaProb = 0.2f;
		in.m_minRoomSize = 0.45f;
		in.m_maxRoomSize = 0.75f;
		in.m_doubleRoomProb = 0.35f;
		in.m_heuristicFactor = 0.4f;
		in.m_pathCostFactor = 0.4f;
		in.m_extraPathCount = 2;
		in.m_extraPathDepth = 2;
		in.m_generateFewerPaths = true;

		return in;
	}

	void Generate(const Input* input, Output* output)
	{
		impl::Generator generator;
		generator.Generate(input, output);
	}
}