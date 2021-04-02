#include "dgen.hpp"

PNode PNode::null = PNode({ 0, 0 });

Dungeon::Dungeon() : cache{}, gInfo(nullptr), pXNodes(nullptr), pYNodes(nullptr) {}
Dungeon::~Dungeon() { Clear(); }

void Dungeon::Prepare()
{
	const size_t toReserve = size_t(1) << (size_t(gInfo -> maxDepth) + 2);

	pXNodes = new std::unordered_multimap<int, PNode*>(toReserve);
	pYNodes = new std::unordered_multimap<int, PNode*>(toReserve);

	pXNodes -> max_load_factor(8);
	pYNodes -> max_load_factor(8);

	tree.Get().space.w = gInfo -> xSize - 3;
	tree.Get().space.h = gInfo -> ySize - 3;

	cache.deltaDepth = gInfo -> maxDepth - gInfo -> minDepth;
	cache.deltaRoomSize = gInfo -> maxRoomSize - gInfo -> minRoomSize;
}

void Dungeon::MakeRoom()
{
	CreateNodes();

	Cell &crrCell = tree.Get();
	SDL_Rect room = crrCell.space;
	SDL_Rect room2;

	bool doubleRoom = (rand() % 100) < gInfo -> doubleRoomProb;

	int dX = int(room.w * ((gInfo -> minRoomSize + rand() % cache.deltaRoomSize) / 100.0f));
	int dY = int(room.h * ((gInfo -> minRoomSize + rand() % cache.deltaRoomSize) / 100.0f));

	room.w -= dX;
	room.h -= dY;

	if (room.w < 4 || room.h < 4) return;

	if (doubleRoom)
	{
		room2 = room;

		if (dX > dY)
		{
			const int extra = int(dX * ((rand() % 100) / 100.0f));

			room2.h >>= 1;
			room2.w += extra;
			dX -= extra;

			if (rand() & 1) room2.y = room.y + room.h - room2.h;
		}
		else
		{
			const int extra = int(dY * ((rand() % 100) / 100.0f));

			room2.w >>= 1;
			room2.h += extra;
			dY -= extra;

			if (rand() & 1) room2.x = room.x + room.w - room2.w;
		}

		if (room2.w < 4 || room2.h < 4) doubleRoom = false;
	}

	const int xOffset = int(dX * (rand() % 100) / 100.0f);
	const int yOffset = int(dY * (rand() % 100) / 100.0f);

	room.x += xOffset;
	room.y += yOffset;

	crrCell.roomList = new Room(room);
	SDL_Rect *selectedRoom = &crrCell.roomList -> room;

	if (doubleRoom)
	{
		room2.x += xOffset;
		room2.y += yOffset;

		crrCell.roomList -> nextRoom = new Room(room2);
		if (rand() & 1) selectedRoom = &crrCell.roomList -> nextRoom -> room;
	}

	retry:
	const bool xMod = rand() & 1;
	const bool xInv = rand() & 1;
	const bool yInv = rand() & 1;

	int x = int(selectedRoom -> x + (selectedRoom -> w - 1) * xInv);
	int y = int(selectedRoom -> y + (selectedRoom -> h - 1) * yInv);

	if (xMod)
	{
		int temp = int(selectedRoom -> w * (rand() % 100) / 100.0f);
		if (xInv) temp = -temp;
		x += temp;
	}
	else
	{
		int temp = int(selectedRoom -> h * (rand() % 100) / 100.0f);
		if (yInv) temp = -temp;
		y += temp;
	}

	SDL_Point entryPoint = { x, y };
	if (doubleRoom)
	{
		SDL_Rect r1 = crrCell.roomList -> room;
		SDL_Rect r2 = crrCell.roomList -> nextRoom -> room;

		r1.w--; r1.h--;
		r2.w--; r2.h--;

		if (IsInside(r1, entryPoint) || IsInside(r2, entryPoint)) goto retry;
	}

	SDL_Point brokerPoint = entryPoint;

	if (xMod)
	{
		if (yInv) brokerPoint.y = crrCell.space.y + crrCell.space.h + 2;
		else brokerPoint.y = crrCell.space.y - 3;
	}
	else
	{
		if (xInv) brokerPoint.x = crrCell.space.x + crrCell.space.w + 2;
		else brokerPoint.x = crrCell.space.x - 3;
	}

	pNodes.push_front(PNode(entryPoint));
	PNode &entry = pNodes.front();
	crrCell.entryNode = &entry;
	PNode &broker = AddNode(brokerPoint.x, brokerPoint.y);

	if (xMod)
	{
		if (yInv)
		{
			entry.links[PNode::SOUTH] = &broker;
			broker.links[PNode::NORTH] = &entry;
		}
		else
		{
			entry.links[PNode::NORTH] = &broker;
			broker.links[PNode::SOUTH] = &entry;
		}

		broker.links[PNode::EAST] = nullptr;
		broker.links[PNode::WEST] = nullptr;
	}
	else
	{
		if (xInv)
		{
			entry.links[PNode::EAST] = &broker;
			broker.links[PNode::WEST] = &entry;
		}
		else
		{
			entry.links[PNode::WEST] = &broker;
			broker.links[PNode::EAST] = &entry;
		}

		broker.links[PNode::NORTH] = nullptr;
		broker.links[PNode::SOUTH] = nullptr;
	}
}

void Dungeon::LinkNodes()
{
	auto Link = [this](PNode &thisNode, int SDL_Point::*pointVar) -> void
	{
		SDL_Point &pos = thisNode.pos;
		int SDL_Point::*opposite = pointVar == &SDL_Point::x ? &SDL_Point::y : &SDL_Point::x;

		PNode *plus = nullptr;
		PNode *minus = nullptr;

		int plusDiff = 0;
		int minusDiff = 0;

		const int val = pos.*pointVar;
		const int oppositeVal = pos.*opposite;

		std::unordered_multimap<int, PNode*> *const map = opposite == &SDL_Point::x ? pXNodes : pYNodes;

		auto range = map -> equal_range(oppositeVal);
		for (auto &i = range.first; i != range.second; i++)
		{
			PNode &node = *(i -> second);
			if (&thisNode == &node) continue;

			const int crrVal = node.pos.*pointVar;
			const int diff = crrVal - val;

			if (diff > 0)
			{
				if (diff < plusDiff || plus == nullptr)
				{
					plus = &node;
					plusDiff = diff;
				}
			}
			else
			{
				if (diff > minusDiff || minus == nullptr)
				{
					minus = &node;
					minusDiff = diff;
				}
			}
		}

		if (pointVar == &SDL_Point::x)
		{
			if (plus != nullptr && thisNode.links[PNode::EAST] == nullptr)
			{
				thisNode.links[PNode::EAST] = plus;
				plus -> links[PNode::WEST] = &thisNode;
			}

			if (minus != nullptr && thisNode.links[PNode::WEST] == nullptr)
			{
				thisNode.links[PNode::WEST] = minus;
				minus -> links[PNode::EAST] = &thisNode;
			}
		}
		else
		{
			if (plus != nullptr && thisNode.links[PNode::SOUTH] == nullptr)
			{
				thisNode.links[PNode::SOUTH] = plus;
				plus -> links[PNode::NORTH] = &thisNode;
			}

			if (minus != nullptr && thisNode.links[PNode::NORTH] == nullptr)
			{
				thisNode.links[PNode::NORTH] = minus;
				minus -> links[PNode::SOUTH] = &thisNode;
			}
		}
	};

	auto end = pNodes.end();
	for (auto iter = pNodes.begin(); iter != end; iter++)
	{
		PNode &node = *iter;
		PNode **links = node.links;

		if (links[PNode::EAST] == nullptr || links[PNode::WEST] == nullptr) Link(node, &SDL_Point::x);
		if (links[PNode::NORTH] == nullptr || links[PNode::SOUTH] == nullptr) Link(node, &SDL_Point::y);
	}

	for (auto iter = pNodes.begin(); iter != end; iter++)
	{
		PNode **links = iter -> links;

		if (links[PNode::NORTH] == &PNode::null) links[PNode::NORTH] = nullptr;
		if (links[PNode::EAST] == &PNode::null) links[PNode::EAST] = nullptr;
		if (links[PNode::SOUTH] == &PNode::null) links[PNode::SOUTH] = nullptr;
		if (links[PNode::WEST] == &PNode::null) links[PNode::WEST] = nullptr;
	}

	delete pYNodes;
	pYNodes = nullptr;

	delete pXNodes;
	pXNodes = nullptr;
}

void Dungeon::CreateNodes()
{
	SDL_Rect &space = tree.Get().space;
	PNode &NW = AddNode(space.x - 3, space.y - 3);
	PNode &NE = AddNode(space.x + space.w + 2, space.y - 3);
	PNode &SW = AddNode(space.x - 3, space.y + space.h + 2);
	PNode &SE = AddNode(space.x + space.w + 2, space.y + space.h + 2);

	NW.links[PNode::EAST] = nullptr;
	NW.links[PNode::SOUTH] = nullptr;

	NE.links[PNode::WEST] = nullptr;
	NE.links[PNode::SOUTH] = nullptr;

	SW.links[PNode::NORTH] = nullptr;
	SW.links[PNode::EAST] = nullptr;

	SE.links[PNode::NORTH] = nullptr;
	SE.links[PNode::WEST] = nullptr;
}

PNode &Dungeon::AddNode(int x, int y)
{
	auto range = pXNodes -> equal_range(x);
	for (auto &i = range.first; i != range.second; i++)
	{
		PNode &node = *(i -> second);
		if (node.pos.y == y) return node;
	}

	pNodes.push_front(PNode({ x, y }));
	PNode &node = pNodes.front();

	pXNodes -> insert(std::make_pair(x, &node));
	pYNodes -> insert(std::make_pair(y, &node));

	return node;
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
		if (rand() % (cache.deltaDepth + 1) >= left) goto nomore;
	}

	left--;

	int SDL_Rect::*xy;
	int SDL_Rect::*wh;

	SDL_Rect *crrSpace = &tree.Get().space;
	const bool vert = crrSpace -> w > crrSpace -> h;

	if (vert) { xy = &SDL_Rect::x; wh = &SDL_Rect::w; }
	else { xy = &SDL_Rect::y; wh = &SDL_Rect::h; }

	const int totalSize = crrSpace ->* wh;
	int randSize = totalSize >> 1;

	if (gInfo -> spaceSizeRandomness > 0) randSize += int(totalSize * ((rand() % gInfo -> spaceSizeRandomness) / 100.0f) - totalSize * ((gInfo -> spaceSizeRandomness >> 1) / 100.0f));

	tree.AddNode(tree.Get());
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

void Dungeon::Clear()
{
	delete pYNodes;
	pYNodes = nullptr;

	delete pXNodes;
	pXNodes = nullptr;

	pNodes.clear();
	tree.Clear();
}

void Dungeon::Generate(GenInfo *genInfo)
{
	ExeHelper<Cell> helper(true, 0, [](const ExeInfo<Cell> &info) -> bool { return info.node.IsLast(); });

	Clear();
	gInfo = genInfo;

	Prepare();
	Divide(gInfo -> maxDepth);

	tree.ExecuteObj(helper, &Dungeon::MakeRoom, this);
	LinkNodes();
}