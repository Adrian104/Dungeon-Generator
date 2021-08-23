#include "pch.hpp"
#include "dgen.hpp"
#include "../logger.hpp"
#include "guard.hpp"

Node Node::refNode = Node(Node::Type::NORMAL);
bool HeapCompare(const std::pair<int, Node*> &p1, const std::pair<int, Node*> &p2) { return p1.first > p2.first; }

Generator::Generator() : roomCount(0), deltaDepth(0), minSpaceSize(0), statusCounter(1), gInput(nullptr), gOutput(nullptr), bValues(0), uniforms{}, root(nullptr) { LOG_HEADER("Dungeon Generator"); }
Generator::~Generator() { Clear(); }

void Generator::Clear()
{
	posYNodes.clear();
	posXNodes.clear();

	openNodes.clear();
	rooms.clear();

	delete root;
	root = nullptr;
}

void Generator::Prepare()
{
	root = new bt::Node<Cell>(nullptr, Cell(gInput -> xSize - 3, gInput -> ySize - 3));

	roomCount = 0;
	statusCounter = 1;
	deltaDepth = gInput -> maxDepth - gInput -> minDepth;
	minSpaceSize = int(400.0f / gInput -> maxRoomSize) + 5;

	uniforms.uni0to99 = std::uniform_int_distribution<int>(0, 99);
	uniforms.uni0to1f = std::uniform_real_distribution<float>(0, 1);
	uniforms.uniDepth = std::uniform_int_distribution<int>(0, deltaDepth);
	uniforms.uniRoom = std::uniform_real_distribution<float>(1.0f - gInput -> maxRoomSize / 100.0f, 1.0f - gInput -> minRoomSize / 100.0f);
	uniforms.uniSpace = std::uniform_real_distribution<float>((gInput -> spaceSizeRandomness >> 1) / -100.0f, (gInput -> spaceSizeRandomness >> 1) / 100.0f);

	bValues = 0;
	RandomBool();
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

		if (links[Dir::SOUTH] == &Node::refNode)
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

		if (links[Dir::EAST] == &Node::refNode)
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
	auto iter = posXNodes.begin();
	auto endIter = posXNodes.end();

	while (iter != endIter)
	{
		Node &node = iter -> second;
		const auto &links = node.links;

		switch (node.path)
		{
		case 0:
			if (links[Dir::NORTH] != nullptr) links[Dir::NORTH] -> links[Dir::SOUTH] = nullptr;
			if (links[Dir::SOUTH] != nullptr) links[Dir::SOUTH] -> links[Dir::NORTH] = nullptr;
			if (links[Dir::WEST] != nullptr) links[Dir::WEST] -> links[Dir::EAST] = nullptr;
			if (links[Dir::EAST] != nullptr) links[Dir::EAST] -> links[Dir::WEST] = nullptr;
			break;

		case 0b0101:
			links[Dir::NORTH] -> links[Dir::SOUTH] = links[Dir::SOUTH];
			links[Dir::SOUTH] -> links[Dir::NORTH] = links[Dir::NORTH];

			if (links[Dir::WEST] != nullptr) links[Dir::WEST] -> links[Dir::EAST] = nullptr;
			if (links[Dir::EAST] != nullptr) links[Dir::EAST] -> links[Dir::WEST] = nullptr;
			break;

		case 0b1010:
			links[Dir::EAST] -> links[Dir::WEST] = links[Dir::WEST];
			links[Dir::WEST] -> links[Dir::EAST] = links[Dir::EAST];

			if (links[Dir::NORTH] != nullptr) links[Dir::NORTH] -> links[Dir::SOUTH] = nullptr;
			if (links[Dir::SOUTH] != nullptr) links[Dir::SOUTH] -> links[Dir::NORTH] = nullptr;
			break;

		default:
			iter++;
			continue;
		}

		iter = posXNodes.erase(iter);
		endIter = posXNodes.end();
	}
}

void Generator::GenerateRooms()
{
	rooms.reserve(roomCount);
	roomCount = 0;

	root -> Execute(bt::Trav::POSTORDER, &Generator::MakeRoom, this, [](const bt::Info<Cell> &info) -> bool { return info.IsLeaf(); });
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

void Generator::GenerateTree(bt::Node<Cell> &btNode, int left)
{
	if (left <= 0)
	{
		nomore:
		Rect &space = btNode.data.space;

		space.x += 4; space.y += 4;
		space.w -= 5; space.h -= 5;

		CreateSpaceNodes(space);
		if (btNode.data.selNode == &Node::refNode)
		{
			if (uniforms.uni0to99(mtEngine) < gInput -> randAreaDens) btNode.data.selNode = nullptr;
			else return;
		}

		roomCount++;
		return;
	}

	if (left <= deltaDepth)
	{
		if (left <= uniforms.uniDepth(mtEngine)) goto nomore;
	}

	if (left <= gInput -> randAreaSize)
	{
		if (uniforms.uni0to99(mtEngine) < gInput -> randAreaProb) btNode.data.selNode = &Node::refNode;
	}

	int Rect::*xy; int Rect::*wh;
	Rect &crrSpace = btNode.data.space;

	if (crrSpace.w < crrSpace.h) { xy = &Rect::y; wh = &Rect::h; }
	else { xy = &Rect::x; wh = &Rect::w; }

	const int totalSize = crrSpace.*wh;
	const int randSize = (totalSize >> 1) + int(totalSize * uniforms.uniSpace(mtEngine));

	if (randSize < minSpaceSize || totalSize - randSize < minSpaceSize) goto nomore;

	btNode.left = new bt::Node<Cell>(&btNode, btNode.data);
	btNode.right = new bt::Node<Cell>(&btNode, btNode.data);

	btNode.left -> data.space.*wh = randSize;
	btNode.right -> data.space.*xy += randSize;
	btNode.right -> data.space.*wh -= randSize;

	left--;

	GenerateTree(*btNode.left, left);
	GenerateTree(*btNode.right, left);

	btNode.data.selNode = nullptr;
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

void Generator::CreateSpaceNodes(Rect &space)
{
	const int xMin = space.x - 3;
	const int xMax = space.x + space.w + 2;

	const int yMin = space.y - 3;
	const int yMax = space.y + space.h + 2;

	Node &NW = AddRegNode(xMin, yMin);
	NW.links[Dir::EAST] = &Node::refNode;
	NW.links[Dir::SOUTH] = &Node::refNode;

	Node &NE = AddRegNode(xMax, yMin);
	NE.links[Dir::SOUTH] = &Node::refNode;

	Node &SW = AddRegNode(xMin, yMax);
	SW.links[Dir::EAST] = &Node::refNode;

	AddRegNode(xMax, yMax);
}

void Generator::CreateRoomNodes(Rect &space, Room &room)
{
	const Point &iPoint = room.iNode.pos;
	int edges[4] = { std::numeric_limits<int>::max(), 0, 0, std::numeric_limits<int>::max() };

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

	Node &iNode = room.iNode;
	Node *const eNodes = room.eNodes;

	iNode.links[Dir::NORTH] = eNodes + Dir::NORTH;
	iNode.links[Dir::EAST] = eNodes + Dir::EAST;
	iNode.links[Dir::SOUTH] = eNodes + Dir::SOUTH;
	iNode.links[Dir::WEST] = eNodes + Dir::WEST;

	Node *const bNodes[4] =
	{
		&AddRegNode(iPoint.x, space.y - 3),
		&AddRegNode(space.x + space.w + 2, iPoint.y),
		&AddRegNode(iPoint.x, space.y + space.h + 2),
		&AddRegNode(space.x - 3, iPoint.y)
	};

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
	bNodes[Dir::NORTH] -> links[Dir::EAST] = &Node::refNode;

	bNodes[Dir::EAST] -> links[Dir::WEST] = &eNodes[Dir::EAST];
	bNodes[Dir::EAST] -> links[Dir::SOUTH] = &Node::refNode;

	bNodes[Dir::SOUTH] -> links[Dir::NORTH] = &eNodes[Dir::SOUTH];
	bNodes[Dir::SOUTH] -> links[Dir::EAST] = &Node::refNode;

	bNodes[Dir::WEST] -> links[Dir::EAST] = &eNodes[Dir::WEST];
	bNodes[Dir::WEST] -> links[Dir::SOUTH] = &Node::refNode;
}

Node *Generator::GetRandomNode(bt::Node<Cell> *const btNode)
{
	if (btNode == nullptr) return nullptr;
	const bool rBool = RandomBool();

	Node *node = GetRandomNode(rBool ? btNode -> left : btNode -> right);
	if (node != nullptr) return node;

	node = GetRandomNode(rBool ? btNode -> right : btNode -> left);
	return node != nullptr ? node : btNode -> data.selNode;
}

void Generator::MakeRoom(bt::Node<Cell> &btNode)
{
	Cell &cell = btNode.data;
	if (cell.selNode == &Node::refNode)
	{
		cell.selNode = nullptr;
		return;
	}

	Rect r1Rect = cell.space;

	int dX = int(r1Rect.w * uniforms.uniRoom(mtEngine));
	int dY = int(r1Rect.h * uniforms.uniRoom(mtEngine));

	r1Rect.w -= dX;
	r1Rect.h -= dY;

	if (r1Rect.w < 4 || r1Rect.h < 4)
	{
		r1Rect = cell.space;

		dX = int(r1Rect.w * uniforms.uniRoom.min());
		dY = int(r1Rect.h * uniforms.uniRoom.min());

		r1Rect.w -= dX;
		r1Rect.h -= dY;
	}

	Rect r2Rect;
	bool doubleRoom = uniforms.uni0to99(mtEngine) < gInput -> doubleRoomProb;

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

	Room &room = rooms.emplace_back();
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

	Rect *const selRoom = rects.data() + mtEngine() % rects.size();

	const int iNodeYPos = selRoom -> y + (mtEngine() % (selRoom -> h - 2)) + 1;
	const int iNodeXPos = selRoom -> x + (mtEngine() % (selRoom -> w - 2)) + 1;

	room.iNode.pos = Point{ iNodeXPos, iNodeYPos };
	cell.selNode = &room.iNode;

	CreateRoomNodes(cell.space, room);
}

void Generator::FindPath(bt::Node<Cell> &btNode)
{
	Node *const start = GetRandomNode(btNode.left);
	if (start == nullptr) return;

	Node *const stop = GetRandomNode(btNode.right);
	if (stop == nullptr || start == stop) return;

	Node *crrNode = start;
	start -> gCost = 0;

	do
	{
		for (Node *&nNode : crrNode -> links)
		{
			if (nNode == nullptr) continue;
			if (nNode -> status > statusCounter) continue;

			int newGCost = crrNode -> gCost;
			if (crrNode -> CheckIfGCost() || nNode -> CheckIfGCost())
			{
				int xDiff = crrNode -> pos.x - nNode -> pos.x;
				int yDiff = crrNode -> pos.y - nNode -> pos.y;

				if (xDiff < 0) xDiff = -xDiff;
				if (yDiff < 0) yDiff = -yDiff;

				newGCost += (xDiff > yDiff) ? xDiff : yDiff;
			}

			if (nNode -> status < statusCounter)
			{
				const Vec diff = stop -> pos - nNode -> pos;

				nNode -> hCost = int(sqrtf(float(diff.x * diff.x + diff.y * diff.y)));
				nNode -> status = statusCounter;
			}
			else
			{
				if (newGCost >= nNode -> gCost) continue;

				const auto endIter = openNodes.end();
				for (auto iter = openNodes.begin(); iter != endIter; iter++)
				{
					if (iter -> second == nNode)
					{
						openNodes.erase(iter);
						break;
					}
				}
			}

			nNode -> prevNode = crrNode;
			nNode -> gCost = newGCost;

			openNodes.push_back(std::make_pair(newGCost + nNode -> hCost, nNode));
			std::push_heap(openNodes.begin(), openNodes.end(), &HeapCompare);
		}

		#ifdef _DEBUG
		if (openNodes.empty()) throw -1;
		#endif

		crrNode -> status = statusCounter + 1;
		crrNode = openNodes.front().second;

		std::pop_heap(openNodes.begin(), openNodes.end(), &HeapCompare);
		openNodes.pop_back();

	} while (crrNode != stop);

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
		else
		{
			crrNode -> path |= 1 << Dir::WEST;
			prevNode -> path |= 1 << Dir::EAST;
		}

		crrNode = prevNode;

	} while (crrNode != start);

	statusCounter += 2;
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

	LOG_TIME("Generating tree");
	GenerateTree(*root, gInput -> maxDepth);

	LOG_TIME("Generating rooms");
	GenerateRooms();

	LOG_TIME("Linking nodes");
	LinkNodes();

	LOG_TIME("Finding paths");
	root -> Execute(bt::Trav::POSTORDER, &Generator::FindPath, this, [](const bt::Info<Cell> &info) -> bool { return info.IsInternal(); });
	for (int i = gInput -> additionalConnections; i > 0; i--) FindPath(*root);

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

	GenerateTree(*root, gInput -> maxDepth);
	GenerateRooms();
	LinkNodes();
	root -> Execute(bt::Trav::POSTORDER, &Generator::FindPath, this, [](const bt::Info<Cell> &info) -> bool { return info.IsInternal(); });
	for (int i = gInput -> additionalConnections; i > 0; i--) FindPath(*root);
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