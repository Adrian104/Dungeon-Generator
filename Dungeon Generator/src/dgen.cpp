#include <algorithm>
#include "dgen.hpp"

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

Dungeon::Dungeon() : cache{}, gInput(nullptr), seed(0) { LOGGER_LOG_HEADER("Dungeon Generator"); }
Dungeon::~Dungeon() { Clear(); }

void Dungeon::MakeRoom()
{
	CreateNodes();

	Cell &crrCell = tree.Get();
	SDL_Rect room = crrCell.space;
	SDL_Rect room2;

	bool doubleRoom = cache.uni0to99(mtEngine) < gInput -> doubleRoomProb;

	int dX = int(room.w * cache.uniRoom(mtEngine));
	int dY = int(room.h * cache.uniRoom(mtEngine));

	room.w -= dX;
	room.h -= dY;

	if (room.w < 4 || room.h < 4) return;

	if (doubleRoom)
	{
		room2 = room;

		if (dX > dY)
		{
			const int extra = int(dX * cache.uni0to1f(mtEngine));

			room2.h >>= 1;
			room2.w += extra;
			dX -= extra;

			if (mtEngine() & 1) room2.y = room.y + room.h - room2.h;
		}
		else
		{
			const int extra = int(dY * cache.uni0to1f(mtEngine));

			room2.w >>= 1;
			room2.h += extra;
			dY -= extra;

			if (mtEngine() & 1) room2.x = room.x + room.w - room2.w;
		}

		if (room2.w < 4 || room2.h < 4) doubleRoom = false;
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
		if (mtEngine() & 1) selRoom = &crrCell.roomList -> nextRoom -> room;
	}

	SDL_Point iPoint = { selRoom -> x, selRoom -> y };
	iPoint.x += mtEngine() % selRoom -> w;
	iPoint.y += mtEngine() % selRoom -> h;

	pNodes.push_front(PNode(iPoint));
	crrCell.internalNode = &pNodes.front();

	AddEntryNode<Dir::NORTH>(crrCell);
	AddEntryNode<Dir::EAST>(crrCell);
	AddEntryNode<Dir::SOUTH>(crrCell);
	AddEntryNode<Dir::WEST>(crrCell);
}

void Dungeon::FindPath()
{
	PNode *start = tree.Left() -> internalNode;
	PNode::stop = tree.Right() -> internalNode;

	const bool startNullptr = start == nullptr;
	const bool stopNullptr = PNode::stop == nullptr;

	if (start == PNode::stop) return;
	PNode **internalNode = &tree.Get().internalNode;

	if (!startNullptr && stopNullptr) { *internalNode = start; return; }
	if (startNullptr && !stopNullptr) { *internalNode = PNode::stop; return; }

	int length = 0, index;
	PNode *crrNode = start;

	do
	{
		for (PNode *&neighbor : crrNode -> links)
		{
			if (neighbor == nullptr) continue;
			if (neighbor -> mode != PNode::CLOSED) neighbor -> Open(crrNode);
		}

		if (openNodes.empty()) goto clear;

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

		length += bool(!(crrNode -> path & PNode::E_NODE));
		crrNode = prevNode;

	} while (crrNode != start);

	length += bool(!(crrNode -> path & PNode::E_NODE));
	index = mtEngine() % length;
	crrNode = PNode::stop;

	while (index > 0)
	{
		index -= bool(!(crrNode -> path & PNode::E_NODE));
		crrNode = crrNode -> prevNode;
	}

	while (crrNode -> path & PNode::E_NODE) crrNode = crrNode -> prevNode;
	*internalNode = crrNode;

	clear:
	PNode::stop -> Reset();

	for (PNode *&node : usedNodes) node -> Reset();
	for (auto &[fCost, node] : openNodes) node -> Reset();

	usedNodes.clear();
	openNodes.clear();
}

void Dungeon::LinkNodes()
{
	PNode *plus;
	PNode *minus;

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
			plus = nullptr;
			minus = nullptr;

			iter = pYNodes.find(std::make_pair(pos.y, pos.x));
			if (iter == yEnd) goto skiplinking;

			crrIter = iter;
			crrIter++;

			if (node.links[Dir::EAST] == nullptr && crrIter != yEnd)
			{
				if (crrIter -> first.first == pos.y)
				{
					plus = crrIter -> second;
					node.links[Dir::EAST] = plus;
					plus -> links[Dir::WEST] = &node;
				}
			}

			crrIter = iter;
			if (node.links[Dir::WEST] == nullptr && crrIter != yBegin)
			{
				crrIter--;
				if (crrIter -> first.first == pos.y)
				{
					minus = crrIter -> second;
					node.links[Dir::WEST] = minus;
					minus -> links[Dir::EAST] = &node;
				}
			}
		}

		if (links[Dir::NORTH] == nullptr || links[Dir::SOUTH] == nullptr)
		{
			plus = nullptr;
			minus = nullptr;

			iter = pXNodes.find(std::make_pair(pos.x, pos.y));
			if (iter == xEnd) goto skiplinking;

			crrIter = iter;
			crrIter++;

			if (node.links[Dir::SOUTH] == nullptr && crrIter != xEnd)
			{
				if (crrIter -> first.first == pos.x)
				{
					plus = crrIter -> second;
					node.links[Dir::SOUTH] = plus;
					plus -> links[Dir::NORTH] = &node;
				}
			}

			crrIter = iter;
			if (node.links[Dir::NORTH] == nullptr && crrIter != xBegin)
			{
				crrIter--;
				if (crrIter -> first.first == pos.x)
				{
					minus = crrIter -> second;
					node.links[Dir::NORTH] = minus;
					minus -> links[Dir::SOUTH] = &node;
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

void Dungeon::CreateNodes()
{
	SDL_Rect &space = tree.Get().space;
	PNode &NW = AddNode(space.x - 3, space.y - 3);
	PNode &NE = AddNode(space.x + space.w + 2, space.y - 3);
	PNode &SW = AddNode(space.x - 3, space.y + space.h + 2);
	PNode &SE = AddNode(space.x + space.w + 2, space.y + space.h + 2);

	NW.links[Dir::EAST] = nullptr;
	NW.links[Dir::SOUTH] = nullptr;

	NE.links[Dir::WEST] = nullptr;
	NE.links[Dir::SOUTH] = nullptr;

	SW.links[Dir::NORTH] = nullptr;
	SW.links[Dir::EAST] = nullptr;

	SE.links[Dir::NORTH] = nullptr;
	SE.links[Dir::WEST] = nullptr;
}

bool Dungeon::Divide(int left)
{
	if (left <= 0)
	{
		nomore:
		SDL_Rect &space = tree.Get().space;

		space.x += 4; space.y += 4;
		space.w -= 5; space.h -= 5;

		return space.w >= 5 && space.h >= 5;
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

	bool ok = Divide(left);

	tree.GoUp();
	tree.GoRight();

	crrSpace = &tree.Get().space;
	crrSpace ->* xy += randSize;
	crrSpace ->* wh -= randSize;

	ok &= Divide(left);
	tree.GoUp();

	if (!ok)
	{
		tree.DeleteNodes();
		goto nomore;
	}

	return true;
}

void Dungeon::Prepare(const bool newSeed)
{
	PNode::heap = &openNodes;

	if (newSeed) seed = rd();
	mtEngine.seed(seed);

	tree.Get().space.w = gInput -> xSize - 3;
	tree.Get().space.h = gInput -> ySize - 3;

	cache.deltaDepth = gInput -> maxDepth - gInput -> minDepth;
	cache.uni0to99 = std::uniform_int_distribution<int>(0, 99);
	cache.uniDepth = std::uniform_int_distribution<int>(0, cache.deltaDepth);

	cache.uni0to1f = std::uniform_real_distribution<float>(0, 1);
	cache.uniRoom = std::uniform_real_distribution<float>(gInput -> minRoomSize / 100.0f, gInput -> maxRoomSize / 100.0f);
	cache.uniSpace = std::uniform_real_distribution<float>((gInput -> spaceSizeRandomness >> 1) / -100.0f, (gInput -> spaceSizeRandomness >> 1) / 100.0f);
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

template <uint8_t dir>
void Dungeon::AddEntryNode(Cell &cell)
{
	PNode &iNode = *cell.internalNode;
	const SDL_Point &iPoint = iNode.pos;

	SDL_Rect *room = &cell.roomList -> room;
	if (Room *nxRoom = cell.roomList -> nextRoom; nxRoom != nullptr)
	{
		SDL_Rect *room2 = &nxRoom -> room;

		if (!SDL_PointInRect(&iNode.pos, room2)) goto skip;
		if (!SDL_PointInRect(&iNode.pos, room)) { room = room2; goto skip; }

		if constexpr (dir == Dir::NORTH)
			{ if (room -> y > room2 -> y) room = room2; }
		else if constexpr (dir == Dir::EAST)
			{ if (room -> x + room -> w < room2 -> x + room2 -> w) room = room2; }
		else if constexpr (dir == Dir::SOUTH)
			{ if (room -> y + room -> h < room2 -> y + room2 -> h) room = room2; }
		else
			{ if (room -> x > room2 -> x) room = room2; }
	}

	skip:
	SDL_Point ePoint = iPoint;
	SDL_Point bPoint;

	if constexpr (dir == Dir::NORTH)
	{
		ePoint.y = room -> y;
		bPoint = { ePoint.x, cell.space.y - 3 };
	}
	else if constexpr (dir == Dir::EAST)
	{
		ePoint.x = room -> x + room -> w - 1;
		bPoint = { cell.space.x + cell.space.w + 2, ePoint.y };
	}
	else if constexpr (dir == Dir::SOUTH)
	{
		ePoint.y = room -> y + room -> h - 1;
		bPoint = { ePoint.x, cell.space.y + cell.space.h + 2 };
	}
	else
	{
		ePoint.x = room -> x;
		bPoint = { cell.space.x - 3, ePoint.y };
	}

	pNodes.push_front(PNode(ePoint));
	PNode &eNode = pNodes.front();
	PNode &bNode = AddNode(bPoint.x, bPoint.y);

	if constexpr (dir == Dir::NORTH)
	{
		iNode.links[Dir::NORTH] = &eNode;
		eNode.links[Dir::NORTH] = &bNode;
		eNode.links[Dir::SOUTH] = &iNode;
		bNode.links[Dir::SOUTH] = &eNode;
	}
	else if constexpr (dir == Dir::EAST)
	{
		iNode.links[Dir::EAST] = &eNode;
		eNode.links[Dir::EAST] = &bNode;
		eNode.links[Dir::WEST] = &iNode;
		bNode.links[Dir::WEST] = &eNode;
	}
	else if constexpr (dir == Dir::SOUTH)
	{
		iNode.links[Dir::SOUTH] = &eNode;
		eNode.links[Dir::SOUTH] = &bNode;
		eNode.links[Dir::NORTH] = &iNode;
		bNode.links[Dir::NORTH] = &eNode;
	}
	else
	{
		iNode.links[Dir::WEST] = &eNode;
		eNode.links[Dir::WEST] = &bNode;
		eNode.links[Dir::EAST] = &iNode;
		bNode.links[Dir::EAST] = &eNode;
	}

	if constexpr (dir == Dir::NORTH || dir == Dir::SOUTH)
	{
		bNode.links[Dir::EAST] = nullptr;
		bNode.links[Dir::WEST] = nullptr;
	}
	else
	{
		bNode.links[Dir::NORTH] = nullptr;
		bNode.links[Dir::SOUTH] = nullptr;
	}

	eNode.path |= PNode::E_NODE;
	iNode.path |= PNode::I_NODE;
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