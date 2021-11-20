#include "pch.hpp"
#include "app.hpp"

inline SDL_FPoint ToFPoint(const Point &point) { return { float(point.x), float(point.y) }; }
inline SDL_FRect ToFRect(const Rect &rect) { return { float(rect.x), float(rect.y), float(rect.w), float(rect.h) }; }

Application::Application() : AppManager(gTitle), plus(false), factor(1), lastFactor(1), gOutput(nullptr)
{
	LoadDefaults();
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, windowWidth, windowHeight);

	overlay = new Overlay(*this);

	overlay -> AddMod(FactorMod("Size factor", factor));
	overlay -> AddMod(IntMod("Minimum depth", gInput.minDepth));
	overlay -> AddMod(IntMod("Maximum depth", gInput.maxDepth));
	overlay -> AddMod(PercentMod("Heuristic", gInput.heuristicFactor));
	overlay -> AddMod(PercentMod("Minimum room size", gInput.minRoomSize));
	overlay -> AddMod(PercentMod("Maximum room size", gInput.maxRoomSize));
	overlay -> AddMod(PercentMod("Space randomness", gInput.spaceSizeRandomness));
	overlay -> AddMod(PercentMod("Double room probability", gInput.doubleRoomProb));
	overlay -> AddMod(IntMod("Additional connections", gInput.additionalConnections));

	overlay -> AddMod(IntMod("Random area depth", gInput.randAreaDepth));
	overlay -> AddMod(PercentMod("Random area density", gInput.randAreaDens));
	overlay -> AddMod(PercentMod("Random area probability", gInput.randAreaProb));

	overlay -> AddMod(BoolMod("Rooms visibility", dInfo.roomsVisibility));
	overlay -> AddMod(BoolMod("Paths visibility", dInfo.pathsVisibility));
	overlay -> AddMod(BoolMod("Entrances visibility", dInfo.entrancesVisibility));
}

Application::~Application()
{
	delete gOutput;
	delete overlay;

	SDL_DestroyTexture(texture);
}

void Application::Run()
{
	if (errorFlag) return;

	Generate(GenMode::NEW_SEED);
	overlay -> Render();

	Render();
	Draw();

	while (Update()) {}
}

void Application::Draw()
{
	SDL_SetRenderTarget(renderer, nullptr);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);

	overlay -> Draw();
	SDL_RenderPresent(renderer);
}

bool Application::Update()
{
	SDL_Event sdlEvent;
	sdlEvent.type = 0;

	bool render = false;
	static bool poll = false;

	if (poll) SDL_PollEvent(&sdlEvent);
	else SDL_WaitEvent(&sdlEvent);

	if (sdlEvent.type == SDL_QUIT) return false;
	if (sdlEvent.type == SDL_KEYDOWN)
	{
		switch (sdlEvent.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			return false;

		case SDLK_g:
			render = true;
			Generate(GenMode::NEW_SEED);
			break;

		case SDLK_r:
			render = true;
			overlay -> refresh = true;

			LoadDefaults();
			Generate(GenMode::OLD_SEED);
			break;

		case SDLK_d:
			Generate(GenMode::DEBUG_MODE);
			Draw();
			break;

		case SDLK_m:
			overlay -> ToggleAnim();
			break;

		case SDLK_UP:
			overlay -> MoveSelected(true);
			break;

		case SDLK_DOWN:
			overlay -> MoveSelected(false);
			break;

		case SDLK_RIGHT:
		case SDLK_RETURN:
		case SDLK_EQUALS:
			if (overlay -> ChangeSelected(false))
			{
				plus = true;
				render = true;
				Generate(GenMode::OLD_SEED);
			}
			break;

		case SDLK_LEFT:
		case SDLK_MINUS:
			if (overlay -> ChangeSelected(true))
			{
				plus = false;
				render = true;
				Generate(GenMode::OLD_SEED);
			}
			break;

		default:
			render |= vPort.Update(sdlEvent);
		}
	}
	else render |= vPort.Update(sdlEvent);

	poll = overlay -> Update();
	if (render)
	{
		Render();
		goto draw;
	}

	if (poll)
	{
		draw:
		Draw();
	}

	return true;
}

void Application::Render()
{
	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	if (dInfo.roomsVisibility)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0xAA, 0xAA, 0xFF);
		for (Rect &room : gOutput -> rooms)
		{
			SDL_FRect rect = ToFRect(room);
			vPort.RectToScreen(rect, rect);
			SDL_RenderDrawRectF(renderer, &rect);
		}
	}

	if (dInfo.pathsVisibility)
	{
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		for (std::pair<Point, Vec> &path : gOutput -> paths)
		{
			SDL_FPoint p1 = ToFPoint(path.first);
			SDL_FPoint p2 = ToFPoint(path.first + path.second);

			p1.x += 0.5f; p1.y += 0.5f;
			p2.x += 0.5f; p2.y += 0.5f;

			vPort.ToScreen(p1.x, p1.y, p1.x, p1.y);
			vPort.ToScreen(p2.x, p2.y, p2.x, p2.y);

			SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
		}
	}

	if (dInfo.entrancesVisibility)
	{
		SDL_SetRenderDrawColor(renderer, 0xFF, 0x60, 0, 0xFF);
		for (Point &entrance : gOutput -> entrances)
		{
			SDL_FPoint point = ToFPoint(entrance);
			SDL_FRect rect = { point.x, point.y, 1, 1 };

			vPort.RectToScreen(rect, rect);
			SDL_RenderFillRectF(renderer, &rect);
		}
	}
}

void Application::RenderDebug()
{
	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	#ifdef SHOW_GRID
	const float scale = vPort.GetScale();

	SDL_FPoint a, b;
	SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 0xFF);

	a = { -vPort.GetXOffset() * scale, -vPort.GetYOffset() * scale };
	b = { a.x, a.y + float(gInput.ySize) * scale };

	for (int x = 0; x <= gInput.xSize; x++)
	{
		SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
		a.x += scale; b.x += scale;
	}

	a = { -vPort.GetXOffset() * scale, -vPort.GetYOffset() * scale };
	b = { a.x + float(gInput.xSize) * scale, a.y };

	for (int y = 0; y <= gInput.ySize; y++)
	{
		SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
		a.y += scale; b.y += scale;
	}
	#endif

	auto DrawSpace = [this](bt::Node<Cell> &btNode) -> void
	{
		SDL_FRect rect = ToFRect(btNode.data.space);

		vPort.RectToScreen(rect, rect);
		SDL_RenderDrawRectF(renderer, &rect);
	};

	const int offset = rand() & 0b10;
	auto DrawLinks = [this, offset](Node &node) -> void
	{
		SDL_FPoint p1 = { float(node.pos.x + 0.5f), float(node.pos.y + 0.5f) };
		vPort.ToScreen(p1.x, p1.y, p1.x, p1.y);

		for (int i = offset; i < offset + 2; i++)
		{
			Node *const node2 = node.links[i];
			if (node2 == nullptr) continue;

			SDL_FPoint p2 = { float(node2 -> pos.x + 0.5f), float(node2 -> pos.y + 0.5f) };

			vPort.ToScreen(p2.x, p2.y, p2.x, p2.y);
			SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
		}
	};

	auto DrawNode = [this](Node &node) -> void
	{
		SDL_FRect rect = { float(node.pos.x), float(node.pos.y), 1, 1 };

		vPort.RectToScreen(rect, rect);
		SDL_RenderFillRectF(renderer, &rect);
	};

	SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
	gen.root -> Execute(bt::Trav::PREORDER, DrawSpace, [](const bt::Info<Cell> &info) -> bool { return info.IsLeaf(); });

	for (Room &room : gen.rooms)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0xAA, 0xAA, 0xFF);
		for (Rect &rect : room.rects)
		{
			SDL_FRect sdlRect = ToFRect(rect);
			vPort.RectToScreen(sdlRect, sdlRect);
			SDL_RenderDrawRectF(renderer, &sdlRect);
		}

		SDL_SetRenderDrawColor(renderer, 0x50, 0x50, 0x50, 0xFF);
		DrawLinks(room);
	}

	SDL_SetRenderDrawColor(renderer, 0x50, 0x50, 0x50, 0xFF);
	for (auto &[pair, node] : gen.posXNodes) DrawLinks(node);

	SDL_SetRenderDrawColor(renderer, 0, 0xC0, 0, 0xFF);
	for (Room &room : gen.rooms) DrawNode(room);

	for (auto &[pair, node] : gen.posXNodes) DrawNode(node);
}

void Application::ApplyFactor()
{
	const float invFactor = 1 / factor;

	gInput.xSize = int(windowWidth * invFactor);
	gInput.ySize = int(windowHeight * invFactor);

	vPort.SetDefaultScale(factor);
	vPort.Reset();

	lastFactor = factor;
}

void Application::LoadDefaults()
{
	gInput.randAreaDens = gDefRandAreaDens;
	gInput.randAreaProb = gDefRandAreaProb;
	gInput.randAreaDepth = gDefRandAreaDepth;

	gInput.xSize = windowWidth;
	gInput.ySize = windowHeight;
	gInput.minDepth = gDefMinDepth;
	gInput.maxDepth = gDefMaxDepth;
	gInput.maxRoomSize = gDefMaxRoomSize;
	gInput.minRoomSize = gDefMinRoomSize;
	gInput.doubleRoomProb = gDefDoubleRoomProb;
	gInput.heuristicFactor = gDefHeuristicFactor;
	gInput.spaceSizeRandomness = gDefSpaceSizeRandomness;
	gInput.additionalConnections = gDefAdditionalConnections;

	dInfo.roomsVisibility = gDefRoomsVisibility;
	dInfo.pathsVisibility = gDefPathsVisibility;
	dInfo.entrancesVisibility = gDefEntrancesVisibility;

	factor = gDefFactor;
	ApplyFactor();
}

void Application::Generate(GenMode mode)
{
	if (lastFactor != factor) ApplyFactor();
	if (gInput.randAreaDepth > gInput.maxDepth) gInput.randAreaDepth = gInput.maxDepth;

	if (gInput.maxDepth < gInput.minDepth)
	{
		if (plus) gInput.maxDepth = gInput.minDepth;
		else gInput.minDepth = gInput.maxDepth;
	}

	if (gInput.maxRoomSize < gInput.minRoomSize)
	{
		if (plus) gInput.maxRoomSize = gInput.minRoomSize;
		else gInput.minRoomSize = gInput.maxRoomSize;
	}

	delete gOutput;
	gOutput = new GenOutput;

	static uint seed = 0;
	if (mode != GenMode::DEBUG_MODE)
	{
		#ifdef RANDOM_SEED
		static std::random_device rd;
		if (mode == GenMode::NEW_SEED) seed = rd();
		#endif

		#ifdef INCREMENTAL_SEED
		seed += mode == GenMode::NEW_SEED;
		#endif

		gen.Generate(&gInput, gOutput, seed);
	}
	else
	{
		MCaller<void, Application> caller(&Application::RenderDebug, this);
		gen.GenerateDebug(&gInput, gOutput, seed, caller);
	}
}