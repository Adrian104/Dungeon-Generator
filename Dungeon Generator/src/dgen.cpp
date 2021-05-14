#include <algorithm>
#include "dgen.hpp"
#include "logger.hpp"

struct HeapCompare
{
	bool operator() (const std::pair<int, PNode*> &p1, const std::pair<int, PNode*> &p2) { return p1.first > p2.first; }
};

PNode PNode::null = PNode({ 0, 0 });
PNode *PNode::stop = nullptr;
std::vector<std::pair<int, PNode*>> *PNode::heap = nullptr;

void PNode::Reset()
{
	gCost = 0;
	mode = UNVISITED;
	prevNode = nullptr;
}

void PNode::Open(PNode *prev)
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
			std::pair<int, PNode*> pair(fCost, this);
			heap -> erase(std::remove_if(heap -> begin(), heap -> end(), [pair](std::pair<int, PNode*> &in) -> bool { return pair.second == in.second; }), heap -> end());
		}

		next:
		prevNode = prev;
		gCost = newGCost;
		fCost = gCost + hCost;

		heap -> push_back(std::make_pair(fCost, this));
		std::push_heap(heap -> begin(), heap -> end(), HeapCompare());
	}
}

Dungeon::Dungeon() : cache{}, gInput(nullptr), seed(0), bValues(0) { LOGGER_LOG_HEADER("Dungeon Generator"); }
Dungeon::~Dungeon() { Clear(); }

void Dungeon::MakeRoom()
{
	CreateSpaceNodes();

	Cell &crrCell = tree.Get();
	SDL_Rect room = crrCell.space;
	SDL_Rect room2;

	bool doubleRoom = cache.uni0to99(mtEngine) < gInput -> doubleRoomProb;

	int dX = int(room.w * cache.uniRoom(mtEngine));
	int dY = int(room.h * cache.uniRoom(mtEngine));

	room.w -= dX;
	room.h -= dY;

	if (room.w < gMinRoomWH || room.h < gMinRoomWH) return;

	if (doubleRoom)
	{
		room2 = room;

		if (dX > dY)
		{
			const int extra = int(dX * cache.uni0to1f(mtEngine));

			room2.h >>= 1;
			room2.w += extra;
			dX -= extra;

			if (RandomBool()) room2.y = room.y + room.h - room2.h;
		}
		else
		{
			const int extra = int(dY * cache.uni0to1f(mtEngine));

			room2.w >>= 1;
			room2.h += extra;
			dY -= extra;

			if (RandomBool()) room2.x = room.x + room.w - room2.w;
		}

		doubleRoom = room2.w >= gMinRoomWH && room2.h >= gMinRoomWH;
	}

	const int xOffset = int(dX * cache.uni0to1f(mtEngine));
	const int yOffset = int(dY * cache.uni0to1f(mtEngine));

	room.x += xOffset;
	room.y += yOffset;

	crrCell.roomList = new Room(room);
	SDL_Rect *selRoom = &crrCell.roomList -> room;

	if (doubleRoom)
	{
		room2.x += xOffset;
		room2.y += yOffset;

		crrCell.roomList -> nextRoom = new Room(room2);
		if (RandomBool()) selRoom = &crrCell.roomList -> nextRoom -> room;
	}

	SDL_Point iPoint = { selRoom -> x, selRoom -> y };
	iPoint.x += mtEngine() % selRoom -> w;
	iPoint.y += mtEngine() % selRoom -> h;

	pNodes.push_front(PNode(iPoint));
	crrCell.internalNode = &pNodes.front();

	CreateRoomNodes();
}

void Dungeon::FindPath()
{
	PNode *start = tree.Left() -> internalNode;
	PNode::stop = tree.Right() -> internalNode;

	const bool startNullptr = start == nullptr;
	const bool stopNullptr = PNode::stop == nullptr;

	if (start == PNode::stop) return;
	PNode **iNode = &tree.Get().internalNode;

	if (!startNullptr && stopNullptr) { *iNode = start; return; }
	if (startNullptr && !stopNullptr) { *iNode = PNode::stop; return; }

	PNode *crrNode = start;

	do
	{
		for (PNode *&neighbor : crrNode -> links)
		{
			if (neighbor == nullptr) continue;
			if (neighbor -> mode != PNode::CLOSED) neighbor -> Open(crrNode);
		}

		if (openNodes.empty()) throw -1;

		crrNode -> mode = PNode::CLOSED;
		usedNodes.push_back(crrNode);

		crrNode = openNodes.at(0).second;
		std::pop_heap(openNodes.begin(), openNodes.end(), HeapCompare());
		openNodes.pop_back();

	} while (crrNode != PNode::stop);

	do
	{
		PNode *prevNode = crrNode -> prevNode;

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

	PNode::stop -> Reset();
	*iNode = RandomBool() ? start : PNode::stop;

	for (PNode *&node : usedNodes) node -> Reset();
	for (auto &[fCost, node] : openNodes) node -> Reset();

	usedNodes.clear();
	openNodes.clear();
}

void Dungeon::LinkNodes()
{
	PNode *xNode;

	std::map<std::pair<int, int>, PNode*>::iterator iter;
	std::map<std::pair<int, int>, PNode*>::iterator crrIter;

	const auto xBegin = pXNodes.begin();
	const auto xEnd = pXNodes.end();

	const auto yBegin = pYNodes.begin();
	const auto yEnd = pYNodes.end();

	for (PNode &node : pNodes)
	{
		PNode **links = node.links;
		const SDL_Point &pos = node.pos;

		if (links[Dir::EAST] == nullptr || links[Dir::WEST] == nullptr)
		{
			iter = pYNodes.find(std::make_pair(pos.y, pos.x));
			if (iter == yEnd) goto skiplinking;

			crrIter = iter;
			crrIter++;

			if (node.links[Dir::EAST] == nullptr && crrIter != yEnd)
			{
				if (crrIter -> first.first == pos.y)
				{
					xNode = crrIter -> second;
					node.links[Dir::EAST] = xNode;
					xNode -> links[Dir::WEST] = &node;
				}
			}

			crrIter = iter;
			if (node.links[Dir::WEST] == nullptr && crrIter != yBegin)
			{
				crrIter--;
				if (crrIter -> first.first == pos.y)
				{
					xNode = crrIter -> second;
					node.links[Dir::WEST] = xNode;
					xNode -> links[Dir::EAST] = &node;
				}
			}
		}

		if (links[Dir::NORTH] == nullptr || links[Dir::SOUTH] == nullptr)
		{
			iter = pXNodes.find(std::make_pair(pos.x, pos.y));
			if (iter == xEnd) goto skiplinking;

			crrIter = iter;
			crrIter++;

			if (node.links[Dir::SOUTH] == nullptr && crrIter != xEnd)
			{
				if (crrIter -> first.first == pos.x)
				{
					xNode = crrIter -> second;
					node.links[Dir::SOUTH] = xNode;
					xNode -> links[Dir::NORTH] = &node;
				}
			}

			crrIter = iter;
			if (node.links[Dir::NORTH] == nullptr && crrIter != xBegin)
			{
				crrIter--;
				if (crrIter -> first.first == pos.x)
				{
					xNode = crrIter -> second;
					node.links[Dir::NORTH] = xNode;
					xNode -> links[Dir::SOUTH] = &node;
				}
			}
		}

		skiplinking:
		if (links[Dir::NORTH] == &PNode::null) links[Dir::NORTH] = nullptr;
		if (links[Dir::EAST] == &PNode::null) links[Dir::EAST] = nullptr;
		if (links[Dir::SOUTH] == &PNode::null) links[Dir::SOUTH] = nullptr;
		if (links[Dir::WEST] == &PNode::null) links[Dir::WEST] = nullptr;
	}
}

bool Dungeon::Divide(int left)
{
	if (left <= 0)
	{
		nomore:
		SDL_Rect &space = tree.Get().space;

		space.x += 4; space.y += 4;
		space.w -= 5; space.h -= 5;

		return space.w < gMinSpaceWH || space.h < gMinSpaceWH;
	}
	else if (left <= cache.deltaDepth)
	{
		if (cache.uniDepth(mtEngine) >= left) goto nomore;
	}

	left--;

	int SDL_Rect::*xy;
	int SDL_Rect::*wh;

	SDL_Rect *crrSpace = &tree.Get().space;
	const bool horizontal = crrSpace -> w < crrSpace -> h;

	if (horizontal) { xy = &SDL_Rect::y; wh = &SDL_Rect::h; }
	else { xy = &SDL_Rect::x; wh = &SDL_Rect::w; }

	const int totalSize = crrSpace ->* wh;
	const int randSize = (totalSize >> 1) + int(totalSize * cache.uniSpace(mtEngine));

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

void Dungeon::Prepare(const bool newSeed)
{
	PNode::heap = &openNodes;

	#ifdef RANDOM_SEED
	if (newSeed) seed = rd();
	#endif

	#ifdef INCREMENTAL_SEED
	if (newSeed) seed++;
	#endif

	mtEngine.seed(seed);

	tree.Get().space.w = gInput -> xSize - 3;
	tree.Get().space.h = gInput -> ySize - 3;

	cache.deltaDepth = gInput -> maxDepth - gInput -> minDepth;
	cache.uni0to99 = std::uniform_int_distribution<int>(0, 99);
	cache.uniDepth = std::uniform_int_distribution<int>(0, cache.deltaDepth);

	cache.uni0to1f = std::uniform_real_distribution<float>(0, 1);
	cache.uniRoom = std::uniform_real_distribution<float>(1.0f - gInput -> maxRoomSize / 100.0f, 1.0f - gInput -> minRoomSize / 100.0f);
	cache.uniSpace = std::uniform_real_distribution<float>((gInput -> spaceSizeRandomness >> 1) / -100.0f, (gInput -> spaceSizeRandomness >> 1) / 100.0f);
	
	bValues = 0;
	RandomBool();
}

bool Dungeon::RandomBool()
{
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

PNode &Dungeon::AddNode(int x, int y)
{
	auto iter = pXNodes.find(std::make_pair(x, y));
	if (iter != pXNodes.end()) return *(iter -> second);

	pNodes.push_front(PNode({ x, y }));
	PNode &node = pNodes.front();

	pXNodes.insert(std::make_pair(std::make_pair(x, y), &node));
	pYNodes.insert(std::make_pair(std::make_pair(y, x), &node));

	return node;
}

void Dungeon::CreateRoomNodes()
{
	Cell &cell = tree.Get();
	PNode &iNode = *cell.internalNode;
	const SDL_Point &iPoint = iNode.pos;

	int edges[4] = { std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() };
	for (Room *crrRoom = cell.roomList; crrRoom != nullptr; crrRoom = crrRoom -> nextRoom)
	{
		const SDL_Rect &rect = crrRoom -> room;

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

	auto FastAddNode = [this](int x, int y) -> PNode*
	{
		pNodes.push_front(PNode({ x, y }));
		return &pNodes.front();
	};

	PNode *const eNodes[4] =
	{
		FastAddNode(iPoint.x, edges[0]),
		FastAddNode(edges[1] - 1, iPoint.y),
		FastAddNode(iPoint.x, edges[2] - 1),
		FastAddNode(edges[3], iPoint.y)
	};

	PNode *const bNodes[4] =
	{
		&AddNode(iPoint.x, cell.space.y - 3),
		&AddNode(cell.space.x + cell.space.w + 2, iPoint.y),
		&AddNode(iPoint.x, cell.space.y + cell.space.h + 2),
		&AddNode(cell.space.x - 3, iPoint.y)
	};

	iNode.path |= PNode::I_NODE;
	iNode.links[Dir::NORTH] = eNodes[Dir::NORTH];
	iNode.links[Dir::EAST] = eNodes[Dir::EAST];
	iNode.links[Dir::SOUTH] = eNodes[Dir::SOUTH];
	iNode.links[Dir::WEST] = eNodes[Dir::WEST];

	eNodes[Dir::NORTH] -> path |= PNode::E_NODE;
	eNodes[Dir::NORTH] -> links[Dir::SOUTH] = &iNode;
	eNodes[Dir::NORTH] -> links[Dir::NORTH] = bNodes[Dir::NORTH];
	eNodes[Dir::EAST] -> path |= PNode::E_NODE;
	eNodes[Dir::EAST] -> links[Dir::WEST] = &iNode;
	eNodes[Dir::EAST] -> links[Dir::EAST] = bNodes[Dir::EAST];
	eNodes[Dir::SOUTH] -> path |= PNode::E_NODE;
	eNodes[Dir::SOUTH] -> links[Dir::NORTH] = &iNode;
	eNodes[Dir::SOUTH] -> links[Dir::SOUTH] = bNodes[Dir::SOUTH];
	eNodes[Dir::WEST] -> path |= PNode::E_NODE;
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

	PNode &NW = AddNode(xMin, yMin);
	PNode &NE = AddNode(xMax, yMin);
	PNode &SW = AddNode(xMin, yMax);
	PNode &SE = AddNode(xMax, yMax);

	NW.links[Dir::EAST] = nullptr;
	NW.links[Dir::SOUTH] = nullptr;

	NE.links[Dir::WEST] = nullptr;
	NE.links[Dir::SOUTH] = nullptr;

	SW.links[Dir::NORTH] = nullptr;
	SW.links[Dir::EAST] = nullptr;

	SE.links[Dir::NORTH] = nullptr;
	SE.links[Dir::WEST] = nullptr;
}

void Dungeon::Clear()
{
	pYNodes.clear();
	pXNodes.clear();

	pNodes.clear();
	usedNodes.clear();
	openNodes.clear();

	tree.Clear();
}

void Dungeon::Generate(GenInput *genInput, const bool newSeed)
{
	LOGGER_LOG("Generation started");
	LOGGER_LOG_ENDL();

	gInput = genInput;
	ExeHelper<Cell> helper(true, 0, [](const ExeInfo<Cell> &info) -> bool { return info.node.IsLast(); });

	LOGGER_LOG_TIME("Preparing");
	Clear();
	Prepare(newSeed);

	LOGGER_LOG_TIME("Partitioning space");
	Divide(gInput -> maxDepth);

	LOGGER_LOG_TIME("Generating rooms");
	tree.ExecuteObj(helper, &Dungeon::MakeRoom, this);

	LOGGER_LOG_TIME("Linking nodes");
	LinkNodes();

	helper.chkFunc = [](const ExeInfo<Cell> &info) -> bool { return !info.node.IsLast(); };

	LOGGER_LOG_TIME("Finding paths");
	tree.ExecuteObj(helper, &Dungeon::FindPath, this);

	LOGGER_LOG("Done!");
	LOGGER_LOG_ENDL();
}