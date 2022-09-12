#include "dgen.hpp"
#include "heap.hpp"

#include <cmath>
#include <stdexcept>

Generator::Generator() : extDist(0), intDist(0), roomCount(0), deltaDepth(0), targetDepth(0), minSpaceSize(0), gOutput(nullptr), gInput(nullptr), root(nullptr) {}
Generator::~Generator() { Clear(); }

void Generator::Clear()
{
	posYNodes.clear();
	posXNodes.clear();
	rooms.clear();

	delete root;
	root = nullptr;
}

void Generator::Prepare()
{
	if (gInput -> m_maxRoomSize <= 0)
		throw std::runtime_error("Variable 'maxRoomSize' is not a positive number");

	*gOutput = {};
	random.Init(gInput -> m_seed);

	extDist = gInput -> m_spaceInterdistance + 1;
	intDist = (extDist << 1) - 1;

	minSpaceSize = static_cast<int>(roomSizeLimit / gInput -> m_maxRoomSize) + intDist;

	if (gInput -> m_width <= minSpaceSize || gInput -> m_height <= minSpaceSize)
		throw std::runtime_error("Root node is too small");

	root = new bt::Node<Cell>(nullptr, Cell(gInput -> m_width - 1, gInput -> m_height - 1));

	roomCount = 0;
	targetDepth = 0;
	deltaDepth = gInput -> m_maxDepth - gInput -> m_minDepth;

	uniSpace = std::uniform_real_distribution<float>(0.5f - gInput -> m_spaceSizeRandomness / 2.0f, 0.5f + gInput -> m_spaceSizeRandomness / 2.0f);
}

void Generator::LinkNodes()
{
	if (posXNodes.empty())
		return;

	{
		const decltype(posXNodes)::iterator end = posXNodes.end();
		decltype(posXNodes)::iterator iter = posXNodes.begin();

		Node* secondary = &(iter -> second);
		while (++iter != end)
		{
			Node* const primary = &(iter -> second);
			if (primary -> path & (1 << Dir::NORTH))
			{
				primary -> links[Dir::NORTH] = secondary;
				secondary -> links[Dir::SOUTH] = primary;
			}

			secondary = primary;
		}
	}

	{
		const decltype(posYNodes)::iterator end = posYNodes.end();
		decltype(posYNodes)::iterator iter = posYNodes.begin();

		Node* secondary = iter -> second;
		while (++iter != end)
		{
			Node* const primary = iter -> second;
			if (secondary -> path & (1 << Dir::EAST))
			{
				secondary -> links[Dir::EAST] = primary;
				primary -> links[Dir::WEST] = secondary;
			}

			secondary -> path = 0;
			secondary = primary;
		}

		secondary -> path = 0;
	}

	posYNodes.clear();
}

void Generator::FindPaths()
{
	uint statusCounter = 1;
	MinHeap<int, Node*> heap;

	bt::Node<Cell>::defaultTraversal = bt::Traversal::POSTORDER;
	for (auto& btNode : *root)
	{
		if (btNode.m_left == nullptr || btNode.m_right == nullptr)
			continue;

		Room *const start = GetRandomRoom(btNode.m_left);
		if (start == nullptr) continue;

		Room *const stop = GetRandomRoom(btNode.m_right);
		if (stop == nullptr) continue;

		Node *crrNode = start;
		start -> gCost = 0;

		do
		{
			crrNode -> status = statusCounter + 1;
			Room *const crrRoom = crrNode -> ToRoom();

			for (int i = 0; i < 4; i++)
			{
				Node *const nNode = crrNode -> links[i];
				if (nNode == nullptr || nNode -> status > statusCounter)
					continue;

				int newGCost = crrNode -> gCost;
				if ((crrNode -> path & (1 << i)) == 0)
				{
					int diff;
					int Point::*const axis = bool(i & 1) ? &Point::x : &Point::y;

					if (crrRoom == nullptr)
					{
						Room *const nRoom = nNode -> ToRoom();

						diff = crrNode -> pos.*axis;
						diff -= (nRoom != nullptr) ? (nRoom -> edges[i ^ 0b10]) : (nNode -> pos.*axis);
					}
					else diff = nNode -> pos.*axis - crrRoom -> edges[i];

					newGCost += diff < 0 ? -diff : diff;
				}

				if (nNode -> status < statusCounter)
				{
					const int dx = stop -> pos.x - nNode -> pos.x;
					const int dy = stop -> pos.y - nNode -> pos.y;

					const float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));

					nNode -> hCost = static_cast<int>(dist * gInput -> m_heuristicFactor);
					nNode -> status = statusCounter;

					goto add_to_heap;
				}

				if (newGCost < nNode -> gCost)
				{
					add_to_heap:
					nNode -> origin = i;
					nNode -> gCost = newGCost;

					heap.Push(newGCost + nNode -> hCost, nNode);
				}
			}

			do
			{
				crrNode = heap.TopObject();
				heap.Pop();

			} while (crrNode -> status > statusCounter);

		} while (crrNode != stop);

		statusCounter += 2;
		heap.Clear();

		do
		{
			const byte origin = crrNode -> origin;
			const byte realOrigin = origin ^ 0b10;

			crrNode -> path |= 1 << realOrigin;
			crrNode = crrNode -> links[realOrigin];
			crrNode -> path |= 1 << origin;

		} while (crrNode != start);
	}
}

void Generator::OptimizeNodes()
{
	auto iter = posXNodes.begin();
	const auto endIter = posXNodes.end();

	const byte maskEW = gInput -> m_generateFewerPaths ? 0b1010 : 0b1111;
	const byte maskNS = gInput -> m_generateFewerPaths ? 0b0101 : 0b1111;

	while (iter != endIter)
	{
		byte& path = iter -> second.path;
		Node** links = iter -> second.links;

		if (path != 0)
		{
			if ((path & maskEW) == 0b1010)
			{
				Node *const east = links[Dir::EAST];
				Node *const west = links[Dir::WEST];

				if (east == nullptr || west == nullptr) goto skip1;
				if (east -> ToRoom() != nullptr && west -> ToRoom() != nullptr) goto skip1;

				east -> links[Dir::WEST] = west;
				west -> links[Dir::EAST] = east;

				links[Dir::EAST] = nullptr;
				links[Dir::WEST] = nullptr;

				path &= ~0b1010;
				if (path == 0) goto zero;
			}

			skip1:
			if ((path & maskNS) == 0b0101)
			{
				Node *const north = links[Dir::NORTH];
				Node *const south = links[Dir::SOUTH];

				if (north == nullptr || south == nullptr) goto skip2;
				if (north -> ToRoom() != nullptr && south -> ToRoom() != nullptr) goto skip2;

				north -> links[Dir::SOUTH] = south;
				south -> links[Dir::NORTH] = north;

				links[Dir::NORTH] = nullptr;
				links[Dir::SOUTH] = nullptr;

				path &= ~0b0101;
				if (path == 0) goto zero;
			}

			skip2:
			iter++;
		}
		else
		{
			zero:
			for (int i = 0; i < 4; i++)
			{
				if (Node *const link = links[i]; link != nullptr)
					link -> links[i ^ 0b10] = nullptr;
			}

			iter = posXNodes.erase(iter);
		}
	}
}

void Generator::GenerateRooms()
{
	std::uniform_real_distribution<float> uniRoom(gInput -> m_minRoomSize, gInput -> m_maxRoomSize);

	rooms.reserve(roomCount);
	bt::Node<Cell>::defaultTraversal = bt::Traversal::POSTORDER;

	for (auto& btNode : *root)
	{
		if (btNode.locked)
			continue;

		Vec priSize(static_cast<int>(btNode.space.w * random(uniRoom)), static_cast<int>(btNode.space.h * random(uniRoom)));

		if (priSize.x < roomSizeLimit)
			priSize.x = roomSizeLimit;

		if (priSize.y < roomSizeLimit)
			priSize.y = roomSizeLimit;

		Vec priPos(btNode.space.x, btNode.space.y);
		Vec remSize(btNode.space.w - priSize.x, btNode.space.h - priSize.y);

		Vec secPos(-1, 0);
		Vec secSize(0, 0);

		if (random.GetFloat() < gInput -> m_doubleRoomProb)
		{
			int Vec::*incAxis; int Vec::*decAxis;

			if (remSize.x > remSize.y) { incAxis = &Vec::x; decAxis = &Vec::y; }
			else { incAxis = &Vec::y; decAxis = &Vec::x; }

			secSize.*decAxis = priSize.*decAxis >> 1;
			if (secSize.*decAxis < roomSizeLimit) goto skip_double_room;

			const int extra = static_cast<int>(remSize.*incAxis * random(uniRoom));
			if (extra <= 0) goto skip_double_room;

			secSize.*incAxis = priSize.*incAxis + extra;
			remSize.*incAxis -= extra;
			secPos = priPos;

			if (const int rem = random() % 3; rem < 2)
				priPos.*incAxis += extra >> rem;

			if (const int rem = random() % 3; rem < 2)
				secPos.*decAxis += (priSize.*decAxis - secSize.*decAxis) >> rem;
		}

		skip_double_room:
		const Vec offset(random() % (remSize.x + 1), random() % (remSize.y + 1));

		Room& room = rooms.emplace_back();
		room.rects.emplace_back(priPos.x + offset.x, priPos.y + offset.y, priSize.x, priSize.y);

		if (secPos.x == -1)
		{
			const Rect& rect = room.rects.front();

			room.pos.x = rect.x + 1 + (random() % (rect.w - 2));
			room.pos.y = rect.y + 1 + (random() % (rect.h - 2));
		}
		else
		{
			room.rects.emplace_back(secPos.x + offset.x, secPos.y + offset.y, secSize.x, secSize.y);
			roomCount++;

			const bool randBool = random.GetBool();
			const Rect& priRect = room.rects.at(static_cast<size_t>(randBool));
			const Rect& secRect = room.rects.at(static_cast<size_t>(!randBool));

			room.pos.x = priRect.x + 1;
			room.pos.y = priRect.y + 1;

			if (priRect.w > secRect.w)
			{
				const int flag1 = static_cast<int>(secRect.x > priRect.x);
				const int flag2 = static_cast<int>(secRect.x + secRect.w < priRect.x + priRect.w);

				room.pos.x += random() % (priRect.w - 2 - flag1 - flag2);
				room.pos.y += random() % (priRect.h - 2);

				if (room.pos.x >= secRect.x)
				{
					room.pos.x += flag1;
					room.pos.x += static_cast<int>(room.pos.x >= secRect.x + secRect.w - 1);
				}
			}
			else
			{
				const int flag1 = static_cast<int>(secRect.y > priRect.y);
				const int flag2 = static_cast<int>(secRect.y + secRect.h < priRect.y + priRect.h);

				room.pos.x += random() % (priRect.w - 2);
				room.pos.y += random() % (priRect.h - 2 - flag1 - flag2);

				if (room.pos.y >= secRect.y)
				{
					room.pos.y += flag1;
					room.pos.y += static_cast<int>(room.pos.y >= secRect.y + secRect.h - 1);
				}
			}
		}

		btNode.room = &room;
		CreateRoomNodes(btNode.space, room);
	}
}

void Generator::GenerateOutput()
{
	gOutput -> m_rooms.reserve(roomCount);

	int c = 0;
	for (Room &room : rooms)
	{
		for (uint path = uint(room.path); path; path >>= 1) c += path & 1;
		for (Rect &rect : room.rects) gOutput -> m_rooms.push_back(rect);
	}

	gOutput -> m_entrances.reserve(c);
	for (auto &[pos, node] : posXNodes)
	{
		if ((node.path & (1 << Dir::NORTH)) != 0) c += node.links[Dir::NORTH] -> ToRoom() == nullptr;
		if ((node.path & (1 << Dir::EAST)) != 0) c += node.links[Dir::EAST] -> ToRoom() == nullptr;
	}

	gOutput -> m_paths.reserve(c);
	for (Room &room : rooms)
	{
		c = 0;
		for (uint path = uint(room.path); path; path >>= 1, c++)
		{
			if (!bool(path & 1)) continue;

			const Point bPos = room.links[c] -> pos;
			const Point ePos = bool(c & 1) ? Point(room.edges[c], bPos.y) : Point(bPos.x, room.edges[c]);

			gOutput -> m_entrances.push_back(ePos);
			gOutput -> m_paths.push_back(std::make_pair(ePos, Vec(bPos.x - ePos.x, bPos.y - ePos.y)));
		}
	}

	for (auto &[pos, node] : posXNodes)
	{
		if ((node.path & (1 << Dir::NORTH)) != 0)
		{
			Node *const nNode = node.links[Dir::NORTH];
			if (nNode -> ToRoom() == nullptr) gOutput -> m_paths.push_back(std::make_pair(node.pos, Vec(nNode -> pos.x - node.pos.x, nNode -> pos.y - node.pos.y)));
		}

		if ((node.path & (1 << Dir::EAST)) != 0)
		{
			Node *const nNode = node.links[Dir::EAST];
			if (nNode -> ToRoom() == nullptr) gOutput -> m_paths.push_back(std::make_pair(node.pos, Vec(nNode -> pos.x - node.pos.x, nNode -> pos.y - node.pos.y)));
		}
	}
}

void Generator::GenerateTree(bt::Node<Cell> &btNode, int left)
{
	if (left <= gInput -> m_randAreaDepth)
		btNode.locked |= random.GetFloat() < gInput -> m_randAreaProb;

	if (left == deltaDepth)
	{
		if (deltaDepth > 0) targetDepth = random() % (deltaDepth + 1);
		else goto no_more;
	}

	if (left <= targetDepth)
	{
		no_more:
		Rect &space = btNode.space;

		space.x += extDist; space.y += extDist;
		space.w -= intDist; space.h -= intDist;

		CreateSpaceNodes(space);
		if (btNode.locked)
		{
			btNode.locked = random.GetFloat() >= gInput -> m_randAreaDens;
			if (btNode.locked) return;
		}

		roomCount++;
		return;
	}

	int Rect::*xy; int Rect::*wh;
	Rect &crrSpace = btNode.space;

	if (crrSpace.w < crrSpace.h) { xy = &Rect::y; wh = &Rect::h; }
	else { xy = &Rect::x; wh = &Rect::w; }

	const int totalSize = crrSpace.*wh;
	const int randSize = static_cast<int>(totalSize * random(uniSpace));

	if (randSize < minSpaceSize || totalSize - randSize < minSpaceSize)
		goto no_more;

	btNode.m_left = new bt::Node<Cell>(&btNode, btNode);
	btNode.m_right = new bt::Node<Cell>(&btNode, btNode);

	btNode.m_left -> space.*wh = randSize;
	btNode.m_right -> space.*xy += randSize;
	btNode.m_right -> space.*wh -= randSize;

	left--;

	GenerateTree(*btNode.m_left, left);
	GenerateTree(*btNode.m_right, left);

	btNode.locked = true;
}

Node &Generator::AddRegNode(int x, int y)
{
	auto pair = posXNodes.emplace(std::make_pair(x, y), Node(x, y));
	if (pair.second) posYNodes.emplace(std::make_pair(y, x), &pair.first -> second);

	return pair.first -> second;
}

void Generator::CreateSpaceNodes(Rect &space)
{
	const int xMin = space.x - extDist;
	const int yMin = space.y - extDist;

	const int dist = extDist - 1;

	const int xMax = space.x + space.w + dist;
	const int yMax = space.y + space.h + dist;

	Node &SW = AddRegNode(xMin, yMax);
	SW.path |= (1 << Dir::NORTH) | (1 << Dir::EAST);

	Node &SE = AddRegNode(xMax, yMax);
	SE.path |= 1 << Dir::NORTH;

	Node &NW = AddRegNode(xMin, yMin);
	NW.path |= 1 << Dir::EAST;

	AddRegNode(xMax, yMin);
}

void Generator::CreateRoomNodes(Rect &space, Room &room)
{
	int *const edges = room.edges;
	const Point iPoint = room.pos;

	for (Rect &rect : room.rects)
	{
		const int xPlusW = rect.x + rect.w;
		const int yPlusH = rect.y + rect.h;

		if (iPoint.x >= rect.x && iPoint.x < xPlusW)
		{
			if (edges[Dir::NORTH] > rect.y) edges[Dir::NORTH] = rect.y;
			if (edges[Dir::SOUTH] < yPlusH) edges[Dir::SOUTH] = yPlusH;
		}

		if (iPoint.y >= rect.y && iPoint.y < yPlusH)
		{
			if (edges[Dir::WEST] > rect.x) edges[Dir::WEST] = rect.x;
			if (edges[Dir::EAST] < xPlusW) edges[Dir::EAST] = xPlusW;
		}
	}

	edges[Dir::EAST]--;
	edges[Dir::SOUTH]--;

	const int dist = extDist - 1;

	Node *const bNodes[4] =
	{
		&AddRegNode(iPoint.x, space.y - extDist),
		&AddRegNode(space.x + space.w + dist, iPoint.y),
		&AddRegNode(iPoint.x, space.y + space.h + dist),
		&AddRegNode(space.x - extDist, iPoint.y)
	};

	for (int i = 0; i < 4; i++)
	{
		Node *const bNode = bNodes[i];
		room.links[i] = bNode;

		bNode -> path |= 0b10 >> (i & 0b1);
		bNode -> links[i ^ 0b10] = &room;
	}
}

Room *Generator::GetRandomRoom(bt::Node<Cell> *const btNode)
{
	if (btNode == nullptr) return nullptr;
	const bool firstLeft = random.GetBool();

	Room *room = GetRandomRoom(firstLeft ? btNode -> m_left : btNode -> m_right);
	if (room != nullptr) return room;

	room = GetRandomRoom(firstLeft ? btNode -> m_right : btNode -> m_left);
	return room != nullptr ? room : btNode -> room;
}

void Generator::Generate(const GenInput *genInput, GenOutput *genOutput)
{
	gInput = genInput;
	gOutput = genOutput;

	Clear();
	Prepare();
	GenerateTree(*root, gInput -> m_maxDepth);
	GenerateRooms();
	LinkNodes();
	FindPaths();
	OptimizeNodes();
	GenerateOutput();
}