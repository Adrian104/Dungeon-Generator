#include "app.hpp"

Application::Application() : quit(false), plus(false), factor(1), lastFactor(1)
{
	LoadDefaults();
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, windowWidth, windowHeight);

	overlay = new Overlay(*this);

	overlay -> AddRef(FactRef("Size factor", &factor));
	overlay -> AddRef(ValRef("Minimum depth", &gInput.minDepth));
	overlay -> AddRef(ValRef("Maximum depth", &gInput.maxDepth));
	overlay -> AddRef(PercRef("Minimum room size", &gInput.minRoomSize));
	overlay -> AddRef(PercRef("Maximum room size", &gInput.maxRoomSize));
	overlay -> AddRef(PercRef("Space randomness", &gInput.spaceSizeRandomness));
	overlay -> AddRef(PercRef("Double room probability", &gInput.doubleRoomProb));
	overlay -> AddRef(BoolRef("Space visibility", &dInfo.spaceVisibility));
	overlay -> AddRef(BoolRef("Rooms visibility", &dInfo.roomsVisibility));
	overlay -> AddRef(ModeRef<3>("Nodes visibility", &dInfo.nodesVisibilityMode, gNodesVisibilityModeNames));
	overlay -> AddRef(ModeRef<3>("Paths visibility", &dInfo.pathsVisibilityMode, gPathsVisibilityModeNames));
}

Application::~Application()
{
	delete overlay;
	SDL_DestroyTexture(texture);
}

void Application::Run()
{
	Generate(true);
	overlay -> Render();

	Render();
	Draw();

	while (!quit)
	{
		Update();

		#ifndef NO_DELAY
		SDL_Delay(1);
		#endif
	}
}

void Application::Draw()
{
	SDL_SetRenderTarget(renderer, nullptr);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);

	overlay -> Draw();
	SDL_RenderPresent(renderer);
}

void Application::Update()
{
	bool refresh = false;
	if (SDL_Event sdlEvent; SDL_PollEvent(&sdlEvent))
	{
		if (sdlEvent.type == SDL_QUIT) quit = true;
		else if (sdlEvent.type == SDL_KEYDOWN)
		{
			switch (sdlEvent.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				quit = true;
				break;

			case SDLK_g:
				refresh = true;
				Generate(true);
				break;

			case SDLK_d:
				refresh = true;
				overlay -> refresh = true;

				LoadDefaults();
				Generate(false);
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

			case SDLK_RETURN:
			case SDLK_EQUALS:
				if (overlay -> ChangeSelected(false))
				{
					plus = true;
					refresh = true;
					Generate(false);
				}
				break;

			case SDLK_MINUS:
				if (overlay -> ChangeSelected(true))
				{
					plus = false;
					refresh = true;
					Generate(false);
				}
				break;

			default:
				refresh = vPort.Update(sdlEvent);
			}
		}
		else refresh = vPort.Update(sdlEvent);
	}

	bool redraw = refresh;
	redraw |= overlay -> Update();

	if (redraw)
	{
		if (refresh) Render();
		Draw();
	}
}

void Application::Render()
{
	ExeHelper<Cell> helper(false, 0, [](const ExeInfo<Cell> &info) -> bool { return info.node.IsLast(); });

	auto DrawSpace = [](Application *mgr) -> void
	{
		SDL_FRect rect = ToFRect(mgr -> dg.tree.Get().space);
		mgr -> vPort.RectToScreen(rect, rect);
		SDL_RenderDrawRectF(mgr -> renderer, &rect);
	};

	auto DrawRooms = [](Application *mgr) -> void
	{
		for (Room *room = mgr -> dg.tree.Get().roomList; room != nullptr; room = room -> nextRoom)
		{
			SDL_FRect rect = ToFRect(room -> room);
			mgr -> vPort.RectToScreen(rect, rect);
			SDL_RenderDrawRectF(mgr -> renderer, &rect);
		}
	};

	auto DrawNodes = [](Application *mgr) -> void
	{
		for (PNode &node : mgr -> dg.pNodes)
		{
			SDL_FPoint point = ToFPoint(node.pos);
			SDL_FRect rect = { point.x, point.y, 1, 1 };

			mgr -> vPort.RectToScreen(rect, rect);
			SDL_RenderFillRectF(mgr -> renderer, &rect);
		}
	};

	auto DrawUsedNodes = [](Application *mgr) -> void
	{
		for (PNode &node : mgr -> dg.pNodes)
		{
			if ((node.path & 0b1111) == 0 || (node.path & PNode::I_NODE)) continue;

			SDL_FPoint point = ToFPoint(node.pos);
			SDL_FRect rect = { point.x, point.y, 1, 1 };

			mgr -> vPort.RectToScreen(rect, rect);
			SDL_RenderFillRectF(mgr -> renderer, &rect);
		}
	};

	auto DrawLinks = [](Application *mgr) -> void
	{
		for (PNode &node : mgr -> dg.pNodes)
		{
			#ifdef RANDOM_COLORS
			int r, g, b;

			do
			{
				r = 255 * (rand() & 0x1);
				g = 255 * (rand() & 0x1);
				b = 255 * (rand() & 0x1);
			} while (r == 0 && g == 0 && b == 0);

			SDL_SetRenderDrawColor(mgr -> renderer, r, g, b, 0xFF);
			#endif

			for (int i = 0; i < 2; i++)
			{
				if (node.links[i] == nullptr) continue;

				SDL_FPoint p1 = ToFPoint(node.pos);
				SDL_FPoint p2 = ToFPoint(node.links[i] -> pos);

				p1.x += 0.5f; p1.y += 0.5f;
				p2.x += 0.5f; p2.y += 0.5f;

				mgr -> vPort.ToScreen(p1.x, p1.y, p1.x, p1.y);
				mgr -> vPort.ToScreen(p2.x, p2.y, p2.x, p2.y);

				SDL_RenderDrawLineF(mgr -> renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}
	};

	auto DrawPaths = [](Application *mgr) -> void
	{
		for (PNode &node : mgr -> dg.pNodes)
		{
			if (node.path & (PNode::E_NODE | PNode::I_NODE)) continue;
			for (int i = 0; i < 4; i++)
			{
				if (!(node.path & (1 << i))) continue;

				SDL_FPoint p1 = ToFPoint(node.pos);
				SDL_FPoint p2 = ToFPoint(node.links[i] -> pos);

				p1.x += 0.5f; p1.y += 0.5f;
				p2.x += 0.5f; p2.y += 0.5f;

				mgr -> vPort.ToScreen(p1.x, p1.y, p1.x, p1.y);
				mgr -> vPort.ToScreen(p2.x, p2.y, p2.x, p2.y);

				SDL_RenderDrawLineF(mgr -> renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}
	};

	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	if (dInfo.spaceVisibility)
	{
		SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
		dg.tree.Execute(helper, &DrawSpace, this);
	}

	if (dInfo.roomsVisibility)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0xAA, 0xAA, 0xFF);
		dg.tree.Execute(helper, &DrawRooms, this);
	}

	if (dInfo.pathsVisibilityMode == 1)
	{
		SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0x55, 0xFF);
		DrawLinks(this);
	}
	else if (dInfo.pathsVisibilityMode == 2)
	{
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		DrawPaths(this);
	}

	if (dInfo.nodesVisibilityMode == 1)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);
		DrawNodes(this);
	}
	else if (dInfo.nodesVisibilityMode == 2)
	{
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xBB, 0, 0xFF);
		DrawUsedNodes(this);
	}

	#ifdef SHOW_GRID
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x16);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	for (float x = 0; x <= gInput.xSize; x++)
	{
		SDL_FPoint a = { x, 0 };
		SDL_FPoint b = { x, float(gInput.ySize) };

		vPort.ToScreen(a.x, a.y, a.x, a.y);
		vPort.ToScreen(b.x, b.y, b.x, b.y);

		SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
	}

	for (float y = 0; y <= gInput.ySize; y++)
	{
		SDL_FPoint a = { 0, y };
		SDL_FPoint b = { float(gInput.xSize), y };

		vPort.ToScreen(a.x, a.y, a.x, a.y);
		vPort.ToScreen(b.x, b.y, b.x, b.y);

		SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
	}
	#endif
}

void Application::ApplyFactor()
{
	gInput.xSize = int(windowWidth * factor);
	gInput.ySize = int(windowHeight * factor);

	vPort.SetDefaultScale(1 / factor);
	vPort.Reset();

	lastFactor = factor;
}

void Application::LoadDefaults()
{
	gInput.xSize = windowWidth;
	gInput.ySize = windowHeight;
	gInput.minDepth = gDefMinDepth;
	gInput.maxDepth = gDefMaxDepth;
	gInput.maxRoomSize = gDefMaxRoomSize;
	gInput.minRoomSize = gDefMinRoomSize;
	gInput.doubleRoomProb = gDefDoubleRoomProb;
	gInput.spaceSizeRandomness = gDefSpaceSizeRandomness;

	dInfo.spaceVisibility = gDefSpaceVisibility;
	dInfo.roomsVisibility = gDefRoomsVisibility;
	dInfo.nodesVisibilityMode = gDefNodesVisibilityMode;
	dInfo.pathsVisibilityMode = gDefPathsVisibilityMode;

	factor = 1;
	ApplyFactor();
}

void Application::Generate(const bool newSeed)
{
	if (lastFactor != factor) ApplyFactor();

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

	dg.Generate(&gInput, newSeed);
}