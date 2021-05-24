#include <algorithm>
#include "dgen.hpp"
#include "logger.hpp"

struct HeapCompare
{
	bool operator() (const std::pair<int, Node*> &p1, const std::pair<int, Node*> &p2) { return p1.first > p2.first; }
};

Node Node::null = Node(0, 0);
Node *Node::stop = nullptr;
std::vector<std::pair<int, Node*>> *Node::heap = nullptr;

void Node::Reset()
{
	gCost = 0;
	mode = UNVISITED;
	prevNode = nullptr;
}

void Node::Open(Node *prev)
{
	int newGCost = prev -> gCost;
	if (prev -> path == 0 || path == 0)
	{
		int xDiff = prev -> pos.x - pos.x;
		int yDiff = prev -> pos.y - pos.y;

		if (xDiff < 0) xDiff = -xDiff;
		if (yDiff < 0) yDiff = -yDiff;

		newGCost += (xDiff > yDiff) ? xDiff : yDiff;
	}

	if (mode == UNVISITED)
	{
		SDL_Point diff = { stop -> pos.x - pos.x, stop -> pos.y - pos.y };
		hCost = int(sqrtf(float(diff.x * diff.x + diff.y * diff.y)));
		mode = OPEN;

		goto next;
	}

	if (newGCost < gCost)
	{
		{
			std::pair<int, Node*> pair(fCost, this);
			heap -> erase(std::remove_if(heap -> begin(), heap -> end(), [pair](std::pair<int, Node*> &in) -> bool { return pair.second == in.second; }), heap -> end());
		}

		next:
		prevNode = prev;
		gCost = newGCost;
		fCost = gCost + hCost;

		heap -> push_back(std::make_pair(fCost, this));
		std::push_heap(heap -> begin(), heap -> end(), HeapCompare());
	}
}

Dungeon::Dungeon() : deltaDepth(0), gInput(nullptr), uniforms{}, bValues(0) { LOGGER_LOG_HEADER("Dungeon Generator"); }
Dungeon::~Dungeon() { Clear(); }

void Dungeon::Prepare()
{
	Clear();

	Node::heap = &openNodes;
	deltaDepth = gInput -> maxDepth - gInput -> minDepth;

	tree.Get().space.w = gInput -> xSize - 3;
	tree.Get().space.h = gInput -> ySize - 3;

	uniforms.uni0to99 = std::uniform_int_distribution<int>(0, 99);
	uniforms.uni0to1f = std::uniform_real_distribution<float>(0, 1);
	uniforms.uniDepth = std::uniform_int_distribution<int>(0, deltaDepth);
	uniforms.uniRoom = std::uniform_real_distribution<float>(1.0f - gInput -> maxRoomSize / 100.0f, 1.0f - gInput -> minRoomSize / 100.0f);
	uniforms.uniSpace = std::uniform_real_distribution<float>((gInput -> spaceSizeRandomness >> 1) / -100.0f, (gInput -> spaceSizeRandomness >> 1) / 100.0f);

	bValues = 0;
	RandomBool();
}

void Dungeon::MakeRoom()
{
	CreateSpaceNodes();

	Cell &crrCell = tree.Get();
	SDL_Rect room = crrCell.space;
	SDL_Rect room2;

	bool doubleRoom = uniforms.uni0to99(mtEngine) < gInput -> doubleRoomProb;

	int dX = int(room.w * uniforms.uniRoom(mtEngine));
	int dY = int(room.h * uniforms.uniRoom(mtEngine));

	room.w -= dX;
	room.h -= dY;

	if (room.w < gMinRoomWH || room.h < gMinRoomWH) return;

	if (doubleRoom)
	{
		room2 = room;

		if (dX > dY)
		{
			const int extra = int(dX * uniforms.uni0to1f(mtEngine));

			room2.h >>= 1;
			room2.w += extra;
			dX -= extra;

			if (RandomBool()) room2.y = room.y + room.h - room2.h;
		}
		else
		{
			const int extra = int(dY * uniforms.uni0to1f(mtEngine));

			room2.w >>= 1;
			room2.h += extra;
			dY -= extra;

			if (RandomBool()) room2.x = room.x + room.w - room2.w;
		}

		doubleRoom = room2.w >= gMinRoomWH && room2.h >= gMinRoomWH;
	}

	const int xOffset = int(dX * uniforms.uni0to1f(mtEngine));
	const int yOffset = int(dY * uniforms.uni0to1f(mtEngine));

	room.x += xOffset;
	room.y += yOffset;

	SDL_Rect *selRoom = &crrCell.roomList.emplace_front(room);
	if (doubleRoom)
	{
		room2.x += xOffset;
		room2.y += yOffset;

		crrCell.roomList.emplace_front(room2);
		if (RandomBool()) selRoom = &crrCell.roomList.front();
	}

	crrCell.internalNode = &nodes.emplace_front(selRoom -> x + (mtEngine() % selRoom -> w), selRoom -> y + (mtEngine() % selRoom -> h));
	CreateRoomNodes();
}

void Dungeon::FindPath()
{
	Node *start = tree.Left() -> internalNode;
	Node::stop = tree.Right() -> internalNode;

	const bool startNullptr = start == nullptr;
	const bool stopNullptr = Node::stop == nullptr;

	if (start == Node::stop) return;
	Node **iNode = &tree.Get().internalNode;

	if (!startNullptr && stopNullptr) { *iNode = start; return; }
	if (startNullptr && !stopNullptr) { *iNode = Node::stop; return; }

	Node *crrNode = start;

	do
	{
		for (Node *&neighbor : crrNode -> links)
		{
			if (neighbor == nullptr) continue;
			if (neighbor -> mode != Node::CLOSED) neighbor -> Open(crrNode);
		}

		if (openNodes.empty()) throw -1;

		crrNode -> mode = Node::CLOSED;
		usedNodes.push_back(crrNode);

		crrNode = openNodes.at(0).second;
		std::pop_heap(openNodes.begin(), openNodes.end(), HeapCompare());
		openNodes.pop_back();

	} while (crrNode != Node::stop);

	do
	{
		Node *prevNode = crrNode -> prevNode;

		int xDiff = prevNode -> pos.x - crrNode -> pos.x;
		int yDiff = prevNode -> pos.y - crrNode -> pos.y;

		if (yDiff < 0)
		{
			crrNode -> path |= 1 << Dir::NORTH;
			prevNode -> path |= 1 << Dir::SOUTH;
		}
		else if (xDiff > 0)
		{
			crrNode -> path |= 1 << Dir::EAST;
			prevNode -> path |= 1 << Dir::WEST;
		}
		else if (yDiff > 0)
		{
			crrNode -> path |= 1 << Dir::SOUTH;
			prevNode -> path |= 1 << Dir::NORTH;
		}
		else if (xDiff < 0)
		{
			crrNode -> path |= 1 << Dir::WEST;
			prevNode -> path |= 1 << Dir::EAST;
		}

		crrNode = prevNode;

	} while (crrNode != start);

	Node::stop -> Reset();
	*iNode = RandomBool() ? start : Node::stop;

	for (Node *&node : usedNodes) node -> Reset();
	for (auto &[fCost, node] : openNodes) node -> Reset();

	usedNodes.clear();
	openNodes.clear();
}

void Dungeon::LinkNodes()
{
	Node *xNode;

	std::map<std::pair<int, int>, Node*>::iterator iter;
	std::map<std::pair<int, int>, Node*>::iterator crrIter;

	const auto xBegin = posXNodes.begin();
	const auto xEnd = posXNodes.end();

	const auto yBegin = posYNodes.begin();
	const auto yEnd = posYNodes.end();

	for (Node &node : nodes)
	{
		Node **links = node.links;
		const SDL_Point &pos = node.pos;

		if (links[Dir::NORTH] == nullptr || links[Dir::SOUTH] == nullptr)
		{
			iter = posXNodes.find(std::make_pair(pos.x, pos.y));
			if (iter == xEnd) goto skiplinking;

			crrIter = iter;
			crrIter++;

			if (links[Dir::SOUTH] == nullptr && crrIter != xEnd)
			{
				if (crrIter -> first.first == pos.x)
				{
					xNode = crrIter -> second;
					links[Dir::SOUTH] = xNode;
					xNode -> links[Dir::NORTH] = &node;
				}
			}

			crrIter = iter;
			if (links[Dir::NORTH] == nullptr && crrIter != xBegin)
			{
				crrIter--;
				if (crrIter -> first.first == pos.x)
				{
					xNode = crrIter -> second;
					links[Dir::NORTH] = xNode;
					xNode -> links[Dir::SOUTH] = &node;
				}
			}
		}

		if (links[Dir::EAST] == nullptr || links[Dir::WEST] == nullptr)
		{
			iter = posYNodes.find(std::make_pair(pos.y, pos.x));
			if (iter == yEnd) goto skiplinking;

			crrIter = iter;
			crrIter++;

			if (links[Dir::EAST] == nullptr && crrIter != yEnd)
			{
				if (crrIter -> first.first == pos.y)
				{
					xNode = crrIter -> second;
					links[Dir::EAST] = xNode;
					xNode -> links[Dir::WEST] = &node;
				}
			}

			crrIter = iter;
			if (links[Dir::WEST] == nullptr && crrIter != yBegin)
			{
				crrIter--;
				if (crrIter -> first.first == pos.y)
				{
					xNode = crrIter -> second;
					links[Dir::WEST] = xNode;
					xNode -> links[Dir::EAST] = &node;
				}
			}
		}

		skiplinking:
		for (auto &link : node.links) { if (link == &Node::null) link = nullptr; }
	}

	posYNodes.clear();
	posXNodes.clear();
}

bool Dungeon::Divide(int left)
{
	if (left <= 0) goto nomore;
	if (left <= deltaDepth)
	{
		if (left <= uniforms.uniDepth(mtEngine))
		{
			nomore:
			SDL_Rect &space = tree.Get().space;

			space.x += 4; space.y += 4;
			space.w -= 5; space.h -= 5;

			return space.w < gMinSpaceWH || space.h < gMinSpaceWH;
		}
	}

	left--;

	int SDL_Rect::*xy;
	int SDL_Rect::*wh;

	SDL_Rect *crrSpace = &tree.Get().space;
	const bool horizontal = crrSpace -> w < crrSpace -> h;

	if (horizontal) { xy = &SDL_Rect::y; wh = &SDL_Rect::h; }
	else { xy = &SDL_Rect::x; wh = &SDL_Rect::w; }

	const int totalSize = crrSpace ->* wh;
	const int randSize = (totalSize >> 1) + int(totalSize * uniforms.uniSpace(mtEngine));

	tree.AddNodes(tree.Get());
	tree.GoLeft();

	crrSpace = &tree.Get().space;
	crrSpace ->* wh = randSize;

	bool notOk = Divide(left);
	
	tree.GoUp();
	tree.GoRight();

	crrSpace = &tree.Get().space;
	crrSpace ->* xy += randSize;
	crrSpace ->* wh -= randSize;

	notOk |= Divide(left);
	tree.GoUp();

	if (notOk)
	{
		tree.DeleteNodes();
		goto nomore;
	}

	return false;
}

bool Dungeon::RandomBool()
{
	static uint32_t randValue = 0;
	const bool ret = randValue & 1;

	if (bValues > 1)
	{
		bValues--;
		randValue >>= 1;
	}
	else
	{
		bValues = 32;
		randValue = mtEngine();
	}

	return ret;
}

void Dungeon::CreateRoomNodes()
{
	Cell &cell = tree.Get();
	Node &iNode = *cell.internalNode;
	const SDL_Point &iPoint = iNode.pos;

	int edges[4] = { std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() };
	for (SDL_Rect &room : cell.roomList)
	{
		const int xPlusW = room.x + room.w;
		const int yPlusH = room.y + room.h;

		if (iPoint.x >= room.x && iPoint.x < xPlusW)
		{
			if (edges[Dir::NORTH] > room.y) edges[Dir::NORTH] = room.y;
			if (edges[Dir::SOUTH] < yPlusH) edges[Dir::SOUTH] = yPlusH;
		}

		if (iPoint.y >= room.y && iPoint.y < yPlusH)
		{
			if (edges[Dir::WEST] > room.x) edges[Dir::WEST] = room.x;
			if (edges[Dir::EAST] < xPlusW) edges[Dir::EAST] = xPlusW;
		}
	}

	Node *const eNodes[4] =
	{
		&nodes.emplace_front(iPoint.x, edges[0]),
		&nodes.emplace_front(edges[1] - 1, iPoint.y),
		&nodes.emplace_front(iPoint.x, edges[2] - 1),
		&nodes.emplace_front(edges[3], iPoint.y)
	};

	Node *const bNodes[4] =
	{
		&AddRegNode(iPoint.x, cell.space.y - 3),
		&AddRegNode(cell.space.x + cell.space.w + 2, iPoint.y),
		&AddRegNode(iPoint.x, cell.space.y + cell.space.h + 2),
		&AddRegNode(cell.space.x - 3, iPoint.y)
	};

	iNode.path |= Node::I_NODE;
	iNode.links[Dir::NORTH] = eNodes[Dir::NORTH];
	iNode.links[Dir::EAST] = eNodes[Dir::EAST];
	iNode.links[Dir::SOUTH] = eNodes[Dir::SOUTH];
	iNode.links[Dir::WEST] = eNodes[Dir::WEST];

	eNodes[Dir::NORTH] -> path |= Node::E_NODE;
	eNodes[Dir::NORTH] -> links[Dir::SOUTH] = &iNode;
	eNodes[Dir::NORTH] -> links[Dir::NORTH] = bNodes[Dir::NORTH];
	eNodes[Dir::EAST] -> path |= Node::E_NODE;
	eNodes[Dir::EAST] -> links[Dir::WEST] = &iNode;
	eNodes[Dir::EAST] -> links[Dir::EAST] = bNodes[Dir::EAST];
	eNodes[Dir::SOUTH] -> path |= Node::E_NODE;
	eNodes[Dir::SOUTH] -> links[Dir::NORTH] = &iNode;
	eNodes[Dir::SOUTH] -> links[Dir::SOUTH] = bNodes[Dir::SOUTH];
	eNodes[Dir::WEST] -> path |= Node::E_NODE;
	eNodes[Dir::WEST] -> links[Dir::EAST] = &iNode;
	eNodes[Dir::WEST] -> links[Dir::WEST] = bNodes[Dir::WEST];

	bNodes[Dir::NORTH] -> links[Dir::SOUTH] = eNodes[Dir::NORTH];
	bNodes[Dir::EAST] -> links[Dir::WEST] = eNodes[Dir::EAST];
	bNodes[Dir::SOUTH] -> links[Dir::NORTH] = eNodes[Dir::SOUTH];
	bNodes[Dir::WEST] -> links[Dir::EAST] = eNodes[Dir::WEST];

	bNodes[Dir::NORTH] -> links[Dir::EAST] = nullptr;
	bNodes[Dir::NORTH] -> links[Dir::WEST] = nullptr;
	bNodes[Dir::EAST] -> links[Dir::NORTH] = nullptr;
	bNodes[Dir::EAST] -> links[Dir::SOUTH] = nullptr;
	bNodes[Dir::SOUTH] -> links[Dir::EAST] = nullptr;
	bNodes[Dir::SOUTH] -> links[Dir::WEST] = nullptr;
	bNodes[Dir::WEST] -> links[Dir::NORTH] = nullptr;
	bNodes[Dir::WEST] -> links[Dir::SOUTH] = nullptr;
}

void Dungeon::CreateSpaceNodes()
{
	SDL_Rect &space = tree.Get().space;

	const int xMin = space.x - 3;
	const int yMin = space.y - 3;

	const int xMax = space.x + space.w + 2;
	const int yMax = space.y + space.h + 2;

	Node &NW = AddRegNode(xMin, yMin);
	Node &NE = AddRegNode(xMax, yMin);
	Node &SW = AddRegNode(xMin, yMax);
	Node &SE = AddRegNode(xMax, yMax);

	NW.links[Dir::EAST] = nullptr;
	NW.links[Dir::SOUTH] = nullptr;

	NE.links[Dir::WEST] = nullptr;
	NE.links[Dir::SOUTH] = nullptr;

	SW.links[Dir::NORTH] = nullptr;
	SW.links[Dir::EAST] = nullptr;

	SE.links[Dir::NORTH] = nullptr;
	SE.links[Dir::WEST] = nullptr;
}

Node &Dungeon::AddRegNode(int x, int y)
{
	const auto iter = posXNodes.find(std::make_pair(x, y));
	if (iter != posXNodes.end()) return *(iter -> second);

	Node &node = nodes.emplace_front(x, y);

	posXNodes.insert(std::make_pair(std::make_pair(x, y), &node));
	posYNodes.insert(std::make_pair(std::make_pair(y, x), &node));

	return node;
}

void Dungeon::Clear()
{
	posYNodes.clear();
	posXNodes.clear();

	nodes.clear();
	usedNodes.clear();
	openNodes.clear();

	tree.Clear();
}

void Dungeon::Generate(GenInput *genInput, const uint32_t seed)
{
	LOGGER_LOG("Generation started");
	LOGGER_LOG_ENDL();

	mtEngine.seed(seed);
	gInput = genInput;

	LOGGER_LOG_TIME("Preparing");
	Prepare();

	LOGGER_LOG_TIME("Partitioning space");
	Divide(gInput -> maxDepth);

	LOGGER_LOG_TIME("Generating rooms");
	tree.ExecuteObj(ExeHelper<Cell>(true, 0, [](const ExeInfo<Cell> &info) -> bool { return info.node.IsLast(); }), &Dungeon::MakeRoom, this);

	LOGGER_LOG_TIME("Linking nodes");
	LinkNodes();

	LOGGER_LOG_TIME("Finding paths");
	tree.ExecuteObj(ExeHelper<Cell>(true, 0, [](const ExeInfo<Cell> &info) -> bool { return !info.node.IsLast(); }), &Dungeon::FindPath, this);

	LOGGER_LOG("Done!");
	LOGGER_LOG_ENDL();
}