#include "pch.hpp"
#include "dgen.hpp"
#include "../logger.hpp"
#include "guard.hpp"

struct HeapCompare
{
	bool operator() (const std::pair<int, Node*> &p1, const std::pair<int, Node*> &p2) { return p1.first > p2.first; }
};

Node *Node::stop = nullptr;
Node Node::reqLink = Node(Node::Type::NORMAL);
std::vector<std::pair<int, Node*>> *Node::heap = nullptr;

void Node::Reset()
{
	gCost = 0;
	mode = Mode::UNVISITED;
}

void Node::Open(Node *prev)
{
	int newGCost = prev -> gCost;
	if (prev -> CheckIfGCost() || CheckIfGCost())
	{
		int xDiff = prev -> pos.x - pos.x;
		int yDiff = prev -> pos.y - pos.y;

		if (xDiff < 0) xDiff = -xDiff;
		if (yDiff < 0) yDiff = -yDiff;

		newGCost += (xDiff > yDiff) ? xDiff : yDiff;
	}

	if (mode == Mode::UNVISITED)
	{
		const Vec diff = stop -> pos - pos;

		hCost = int(sqrtf(float(diff.x * diff.x + diff.y * diff.y)));
		mode = Mode::OPEN;

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

Generator::Generator() : roomCount(0), deltaDepth(0), gInput(nullptr), gOutput(nullptr), bValues(0), uniforms{}, root(nullptr) { LOG_HEADER("Dungeon Generator"); }
Generator::~Generator() { Clear(); }

void Generator::Clear()
{
	posYNodes.clear();
	posXNodes.clear();

	usedNodes.clear();
	openNodes.clear();
	rooms.clear();

	delete root;
	root = nullptr;
}

void Generator::Prepare()
{
	root = new bt::Node<Cell>(nullptr, Cell(gInput -> xSize - 3, gInput -> ySize - 3));

	roomCount = 0;
	deltaDepth = gInput -> maxDepth - gInput -> minDepth;

	uniforms.uni0to99 = std::uniform_int_distribution<int>(0, 99);
	uniforms.uni0to1f = std::uniform_real_distribution<float>(0, 1);
	uniforms.uniDepth = std::uniform_int_distribution<int>(0, deltaDepth);
	uniforms.uniRoom = std::uniform_real_distribution<float>(1.0f - gInput -> maxRoomSize / 100.0f, 1.0f - gInput -> minRoomSize / 100.0f);
	uniforms.uniSpace = std::uniform_real_distribution<float>((gInput -> spaceSizeRandomness >> 1) / -100.0f, (gInput -> spaceSizeRandomness >> 1) / 100.0f);

	bValues = 0;
	RandomBool();

	Node::heap = &openNodes;
}

void Generator::LinkNodes()
{
	std::map<std::pair<int, int>, Node>::iterator crrXIter;
	std::map<std::pair<int, int>, Node>::iterator nextXIter;

	crrXIter = posXNodes.begin();
	nextXIter = crrXIter;
	nextXIter++;

	const auto endXIter = posXNodes.end();
	while (nextXIter != endXIter)
	{
		Node *const node = &crrXIter -> second;
		Node **links = node -> links;

		if (links[Dir::SOUTH] == &Node::reqLink)
		{
			Node *const nextNode = &nextXIter -> second;
			links[Dir::SOUTH] = nextNode;
			nextNode -> links[Dir::NORTH] = node;
		}

		crrXIter = nextXIter;
		nextXIter++;
	}

	std::map<std::pair<int, int>, Node*>::iterator crrYIter;
	std::map<std::pair<int, int>, Node*>::iterator nextYIter;

	crrYIter = posYNodes.begin();
	nextYIter = crrYIter;
	nextYIter++;

	const auto endYIter = posYNodes.end();
	while (nextYIter != endYIter)
	{
		Node *const node = crrYIter -> second;
		Node **links = node -> links;

		if (links[Dir::EAST] == &Node::reqLink)
		{
			Node *const nextNode = nextYIter -> second;
			links[Dir::EAST] = nextNode;
			nextNode -> links[Dir::WEST] = node;
		}

		crrYIter = nextYIter;
		nextYIter++;
	}

	posYNodes.clear();
}

void Generator::OptimizeNodes()
{
	for (auto iter = posXNodes.begin(); iter != posXNodes.end(); iter++)
	{
		rep:
		Node &node = iter -> second;

		if (node.path == 0b0101)
		{
			node.links[Dir::NORTH] -> links[Dir::SOUTH] = node.links[Dir::SOUTH];
			node.links[Dir::SOUTH] -> links[Dir::NORTH] = node.links[Dir::NORTH];

			if (node.links[Dir::WEST] != nullptr) node.links[Dir::WEST] -> links[Dir::EAST] = nullptr;
			if (node.links[Dir::EAST] != nullptr) node.links[Dir::EAST] -> links[Dir::WEST] = nullptr;
		}
		else if (node.path == 0b1010)
		{
			node.links[Dir::EAST] -> links[Dir::WEST] = node.links[Dir::WEST];
			node.links[Dir::WEST] -> links[Dir::EAST] = node.links[Dir::EAST];

			if (node.links[Dir::NORTH] != nullptr) node.links[Dir::NORTH] -> links[Dir::SOUTH] = nullptr;
			if (node.links[Dir::SOUTH] != nullptr) node.links[Dir::SOUTH] -> links[Dir::NORTH] = nullptr;
		}
		else if (node.path == 0)
		{
			if (node.links[Dir::NORTH] != nullptr) node.links[Dir::NORTH] -> links[Dir::SOUTH] = nullptr;
			if (node.links[Dir::SOUTH] != nullptr) node.links[Dir::SOUTH] -> links[Dir::NORTH] = nullptr;
			if (node.links[Dir::WEST] != nullptr) node.links[Dir::WEST] -> links[Dir::EAST] = nullptr;
			if (node.links[Dir::EAST] != nullptr) node.links[Dir::EAST] -> links[Dir::WEST] = nullptr;
		}
		else continue;

		iter = posXNodes.erase(iter);
		if (iter == posXNodes.end()) break;
		goto rep;
	}
}

void Generator::GenerateOutput()
{
	int eCount = 0;
	int pCount = 0;

	gOutput -> rooms.reserve(roomCount);
	for (Room &room : rooms)
	{
		pCount += (room.eNodes[Dir::NORTH].path & (1 << Dir::NORTH)) != 0;
		pCount += (room.eNodes[Dir::EAST].path & (1 << Dir::EAST)) != 0;

		for (Node &node : room.eNodes) eCount += node.path != 0;
		for (Rect &rect : room.rects) gOutput -> rooms.push_back(rect);
	}

	for (auto &[pair, node] : posXNodes)
	{
		pCount += (node.path & (1 << Dir::NORTH)) != 0;
		pCount += (node.path & (1 << Dir::EAST)) != 0;
	}

	gOutput -> paths.reserve(pCount);
	gOutput -> entrances.reserve(eCount);

	for (Room &room : rooms)
	{
		if (Node &node = room.eNodes[Dir::NORTH]; node.path & (1 << Dir::NORTH)) gOutput -> paths.push_back(std::make_pair(node.pos, node.links[Dir::NORTH] -> pos - node.pos));
		if (Node &node = room.eNodes[Dir::EAST]; node.path & (1 << Dir::EAST)) gOutput -> paths.push_back(std::make_pair(node.pos, node.links[Dir::EAST] -> pos - node.pos));

		for (Node &node : room.eNodes)
		{
			if (node.path != 0) gOutput -> entrances.push_back(node.pos);
		}
	}

	for (auto &[pair, node] : posXNodes)
	{
		if (node.path & (1 << Dir::NORTH)) gOutput -> paths.push_back(std::make_pair(node.pos, node.links[Dir::NORTH] -> pos - node.pos));
		if (node.path & (1 << Dir::EAST)) gOutput -> paths.push_back(std::make_pair(node.pos, node.links[Dir::EAST] -> pos - node.pos));
	}
}

bool Generator::Divide(bt::Node<Cell> &btNode, int left)
{
	if (left <= 0) goto nomore;
	if (left <= deltaDepth)
	{
		if (left <= uniforms.uniDepth(mtEngine))
		{
			nomore:
			Rect &space = btNode.data.space;

			space.x += 4; space.y += 4;
			space.w -= 5; space.h -= 5;

			return space.w < 5 || space.h < 5;
		}
	}

	left--;

	int Rect::*xy;
	int Rect::*wh;

	Rect *crrSpace = &btNode.data.space;
	const bool horizontal = crrSpace -> w < crrSpace -> h;

	if (horizontal) { xy = &Rect::y; wh = &Rect::h; }
	else { xy = &Rect::x; wh = &Rect::w; }

	const int totalSize = crrSpace ->* wh;
	const int randSize = (totalSize >> 1) + int(totalSize * uniforms.uniSpace(mtEngine));

	btNode.left = new bt::Node<Cell>(&btNode, btNode.data);
	btNode.left -> data.space.*wh = randSize;

	bool notOk = Divide(*btNode.left, left);

	btNode.right = new bt::Node<Cell>(&btNode, btNode.data);
	crrSpace = &btNode.right -> data.space;

	crrSpace ->* xy += randSize;
	crrSpace ->* wh -= randSize;

	notOk |= Divide(*btNode.right, left);

	if (notOk)
	{
		delete btNode.right;
		btNode.right = nullptr;

		delete btNode.left;
		btNode.left = nullptr;

		goto nomore;
	}

	return false;
}

bool Generator::RandomBool()
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

Node &Generator::AddRegNode(int x, int y)
{
	auto pair = posXNodes.emplace(std::make_pair(x, y), Node(Node::Type::NORMAL, x, y));
	if (pair.second) posYNodes.emplace(std::make_pair(y, x), &pair.first -> second);

	return pair.first -> second;
}

void Generator::CreateSpaceNodes(Cell &cell)
{
	Rect &space = cell.space;

	const int xMin = space.x - 3;
	const int yMin = space.y - 3;

	const int xMax = space.x + space.w + 2;
	const int yMax = space.y + space.h + 2;

	Node &NW = AddRegNode(xMin, yMin);
	Node &NE = AddRegNode(xMax, yMin);
	Node &SW = AddRegNode(xMin, yMax);
	Node &SE = AddRegNode(xMax, yMax);

	NW.links[Dir::EAST] = &Node::reqLink;
	NW.links[Dir::SOUTH] = &Node::reqLink;
	NE.links[Dir::SOUTH] = &Node::reqLink;
	SW.links[Dir::EAST] = &Node::reqLink;
}

void Generator::CreateRoomNodes(Cell &cell, Room &room)
{
	Node &iNode = room.iNode;
	const Point &iPoint = iNode.pos;

	int edges[4] = { std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() };
	for (Rect &room : room.rects)
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

	Node *const bNodes[4] =
	{
		&AddRegNode(iPoint.x, cell.space.y - 3),
		&AddRegNode(cell.space.x + cell.space.w + 2, iPoint.y),
		&AddRegNode(iPoint.x, cell.space.y + cell.space.h + 2),
		&AddRegNode(cell.space.x - 3, iPoint.y)
	};

	Node *const eNodes = room.eNodes;

	iNode.links[Dir::NORTH] = &eNodes[Dir::NORTH];
	iNode.links[Dir::EAST] = &eNodes[Dir::EAST];
	iNode.links[Dir::SOUTH] = &eNodes[Dir::SOUTH];
	iNode.links[Dir::WEST] = &eNodes[Dir::WEST];

	eNodes[Dir::NORTH].links[Dir::SOUTH] = &iNode;
	eNodes[Dir::NORTH].links[Dir::NORTH] = bNodes[Dir::NORTH];
	eNodes[Dir::NORTH].pos = Point{ iPoint.x, edges[0] };

	eNodes[Dir::EAST].links[Dir::WEST] = &iNode;
	eNodes[Dir::EAST].links[Dir::EAST] = bNodes[Dir::EAST];
	eNodes[Dir::EAST].pos = Point{ edges[1] - 1, iPoint.y };

	eNodes[Dir::SOUTH].links[Dir::NORTH] = &iNode;
	eNodes[Dir::SOUTH].links[Dir::SOUTH] = bNodes[Dir::SOUTH];
	eNodes[Dir::SOUTH].pos = Point{ iPoint.x, edges[2] - 1 };

	eNodes[Dir::WEST].links[Dir::EAST] = &iNode;
	eNodes[Dir::WEST].links[Dir::WEST] = bNodes[Dir::WEST];
	eNodes[Dir::WEST].pos = Point{ edges[3], iPoint.y };

	bNodes[Dir::NORTH] -> links[Dir::SOUTH] = &eNodes[Dir::NORTH];
	bNodes[Dir::EAST] -> links[Dir::WEST] = &eNodes[Dir::EAST];
	bNodes[Dir::SOUTH] -> links[Dir::NORTH] = &eNodes[Dir::SOUTH];
	bNodes[Dir::WEST] -> links[Dir::EAST] = &eNodes[Dir::WEST];

	bNodes[Dir::NORTH] -> links[Dir::EAST] = &Node::reqLink;
	bNodes[Dir::EAST] -> links[Dir::SOUTH] = &Node::reqLink;
	bNodes[Dir::SOUTH] -> links[Dir::EAST] = &Node::reqLink;
	bNodes[Dir::WEST] -> links[Dir::SOUTH] = &Node::reqLink;
}

void Generator::MakeRoom(bt::Node<Cell> &btNode)
{
	Cell &cell = btNode.data;
	CreateSpaceNodes(cell);

	Rect r1Rect = cell.space;
	Rect r2Rect;

	bool doubleRoom = uniforms.uni0to99(mtEngine) < gInput -> doubleRoomProb;

	int dX = int(r1Rect.w * uniforms.uniRoom(mtEngine));
	int dY = int(r1Rect.h * uniforms.uniRoom(mtEngine));

	r1Rect.w -= dX;
	r1Rect.h -= dY;

	if (r1Rect.w < 4 || r1Rect.h < 4) return;

	if (doubleRoom)
	{
		r2Rect = r1Rect;

		if (dX > dY)
		{
			const int extra = int(dX * uniforms.uni0to1f(mtEngine));

			r2Rect.h >>= 1;
			r2Rect.w += extra;
			dX -= extra;

			if (RandomBool()) r2Rect.y = r1Rect.y + r1Rect.h - r2Rect.h;
		}
		else
		{
			const int extra = int(dY * uniforms.uni0to1f(mtEngine));

			r2Rect.w >>= 1;
			r2Rect.h += extra;
			dY -= extra;

			if (RandomBool()) r2Rect.x = r1Rect.x + r1Rect.w - r2Rect.w;
		}

		doubleRoom = r2Rect.w >= 4 && r2Rect.h >= 4;
	}

	const int xOffset = int(dX * uniforms.uni0to1f(mtEngine));
	const int yOffset = int(dY * uniforms.uni0to1f(mtEngine));

	r1Rect.x += xOffset;
	r1Rect.y += yOffset;

	Room &room = rooms.emplace_front();
	std::vector<Rect> &rects = room.rects;

	rects.emplace_back(r1Rect);
	roomCount++;

	if (doubleRoom)
	{
		r2Rect.x += xOffset;
		r2Rect.y += yOffset;

		rects.emplace_back(r2Rect);
		roomCount++;
	}

	Rect *const selRoom = &rects.at(mtEngine() % rects.size());

	const int iNodeYPos = selRoom -> y + (mtEngine() % (selRoom -> h - 2)) + 1;
	const int iNodeXPos = selRoom -> x + (mtEngine() % (selRoom -> w - 2)) + 1;

	room.iNode.pos = Point{ iNodeXPos, iNodeYPos };
	cell.selNode = &room.iNode;

	CreateRoomNodes(cell, room);
}

void Generator::FindPath(bt::Node<Cell> &btNode)
{
	Node *start = btNode.left -> data.selNode;
	Node::stop = btNode.right -> data.selNode;

	const bool startNullptr = start == nullptr;
	const bool stopNullptr = Node::stop == nullptr;

	if (start == Node::stop) return;
	Node **iNode = &btNode.data.selNode;

	if (!startNullptr && stopNullptr) { *iNode = start; return; }
	if (startNullptr && !stopNullptr) { *iNode = Node::stop; return; }

	Node *crrNode = start;

	do
	{
		for (Node *&neighbor : crrNode -> links)
		{
			if (neighbor == nullptr) continue;
			if (neighbor -> mode != Node::Mode::CLOSED) neighbor -> Open(crrNode);
		}

		#ifdef _DEBUG
		if (openNodes.empty()) throw -1;
		#endif

		crrNode -> mode = Node::Mode::CLOSED;
		usedNodes.push_back(crrNode);

		crrNode = openNodes.at(0).second;
		std::pop_heap(openNodes.begin(), openNodes.end(), HeapCompare());
		openNodes.pop_back();

	} while (crrNode != Node::stop);

	do
	{
		Node **links = crrNode -> links;
		Node *const prevNode = crrNode -> prevNode;

		if (prevNode == links[0])
		{
			crrNode -> path |= 1 << Dir::NORTH;
			prevNode -> path |= 1 << Dir::SOUTH;
		}
		else if (prevNode == links[1])
		{
			crrNode -> path |= 1 << Dir::EAST;
			prevNode -> path |= 1 << Dir::WEST;
		}
		else if (prevNode == links[2])
		{
			crrNode -> path |= 1 << Dir::SOUTH;
			prevNode -> path |= 1 << Dir::NORTH;
		}
		else if (prevNode == links[3])
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

void Generator::Generate(GenInput *genInput, GenOutput *genOutput, const uint32_t seed)
{
	LOG_MSG("Generation started");
	LOG_ENDL();

	gInput = genInput;
	gOutput = genOutput;

	mtEngine.seed(seed);
	Prepare();

	LOG_TIME("Partitioning space");
	Divide(*root, gInput -> maxDepth);

	LOG_TIME("Generating rooms");
	root -> Execute(bt::Trav::POSTORDER, &Generator::MakeRoom, this, [](const bt::Info<Cell> &info) -> bool { return info.IsLeaf(); });

	LOG_TIME("Linking nodes");
	LinkNodes();

	LOG_TIME("Finding paths");
	root -> Execute(bt::Trav::POSTORDER, &Generator::FindPath, this, [](const bt::Info<Cell> &info) -> bool { return info.IsInternal(); });

	LOG_TIME("Optimizing nodes");
	OptimizeNodes();

	LOG_TIME("Generating output");
	GenerateOutput();

	LOG_TIME("Deallocating memory");
	Clear();

	LOG_ENDL();
	LOG_MSG("Done!");
	LOG_TOTAL_TIME("Total time: ");
	LOG_ENDL();
}

void Generator::GenerateDebug(GenInput *genInput, GenOutput *genOutput, const uint32_t seed, Caller<void> &callback)
{
	LOG_MSG("Generation started (with debugging)");
	LOG_ENDL();

	gInput = genInput;
	gOutput = genOutput;

	mtEngine.seed(seed);
	Prepare();

	LOG_TIME("Preparing for debugging");

	Divide(*root, gInput -> maxDepth);
	root -> Execute(bt::Trav::POSTORDER, &Generator::MakeRoom, this, [](const bt::Info<Cell> &info) -> bool { return info.IsLeaf(); });
	LinkNodes();
	root -> Execute(bt::Trav::POSTORDER, &Generator::FindPath, this, [](const bt::Info<Cell> &info) -> bool { return info.IsInternal(); });
	OptimizeNodes();

	LOG_TIME("Rendering objects");
	callback.Call();

	LOG_TIME("Continuing work");

	GenerateOutput();
	Clear();

	LOG_ENDL();
	LOG_MSG("Done!");
	LOG_TOTAL_TIME("Total time: ");
	LOG_ENDL();
}