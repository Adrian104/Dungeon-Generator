#include <iostream>
#include <cmath>
#include <random>
#include <stdexcept>
#include "app.hpp"

void RenderDebugHelper(Application *app) { app -> RenderDebug(); }

inline SDL_FPoint ToFPoint(const Point &point) { return { float(point.x), float(point.y) }; }
inline SDL_FRect ToFRect(const Rect &rect) { return { float(rect.x), float(rect.y), float(rect.w), float(rect.h) }; }

Application::Application() : plus(false), factor(1), lastFactor(1)
{
	for (int i = 0; i < sizeof(gFonts) / sizeof(*gFonts); i++)
	{
		const auto& font = gFonts[i];
		LoadFont(i, font.second, font.first);
	}

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	#ifdef FULL_SCREEN
		CreateWindow(gTitle, dm.w, dm.h, SDL_WINDOW_HIDDEN | SDL_WINDOW_FULLSCREEN);
	#else
		CreateWindow(gTitle, dm.w - 30, dm.h - 100, SDL_WINDOW_HIDDEN);
	#endif

	LoadDefaults();
	texture = SDL_CreateTexture(GetRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GetSize().x, GetSize().y);

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

	SDL_ShowWindow(GetWindow());
}

Application::~Application()
{
	delete overlay;
	SDL_DestroyTexture(texture);
}

void Application::Run()
{
	Generate(GenMode::NEW_SEED);
	overlay -> Render();

	Render();
	Draw();

	while (Update()) {}
}

void Application::Draw()
{
	SDL_SetRenderTarget(GetRenderer(), nullptr);
	SDL_RenderCopy(GetRenderer(), texture, nullptr, nullptr);

	overlay -> Draw();
	SDL_RenderPresent(GetRenderer());
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

		case SDLK_TAB:
			render = true;
			vPort.Reset();
			break;

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
			overlay -> Play(Animator::DirMode::AUTO);
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
	RenderCommon();

	if (dInfo.roomsVisibility)
	{
		SDL_SetRenderDrawColor(GetRenderer(), 0, 0xAA, 0xAA, 0xFF);
		for (Rect &room : gOutput.rooms)
		{
			SDL_FRect rect = ToFRect(room);
			vPort.RectToScreen(rect, rect);
			SDL_RenderDrawRectF(GetRenderer(), &rect);
		}
	}

	if (dInfo.pathsVisibility)
	{
		SDL_SetRenderDrawColor(GetRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);
		for (std::pair<Point, Vec> &path : gOutput.paths)
		{
			SDL_FPoint p1 = ToFPoint(path.first);
			SDL_FPoint p2 = ToFPoint(path.first + path.second);

			p1.x += 0.5f; p1.y += 0.5f;
			p2.x += 0.5f; p2.y += 0.5f;

			vPort.ToScreen(p1.x, p1.y, p1.x, p1.y);
			vPort.ToScreen(p2.x, p2.y, p2.x, p2.y);

			SDL_RenderDrawLineF(GetRenderer(), p1.x, p1.y, p2.x, p2.y);
		}
	}

	if (dInfo.entrancesVisibility)
	{
		SDL_SetRenderDrawColor(GetRenderer(), 0xFF, 0x60, 0, 0xFF);
		for (Point &entrance : gOutput.entrances)
		{
			SDL_FPoint point = ToFPoint(entrance);
			SDL_FRect rect = { point.x, point.y, 1, 1 };

			vPort.RectToScreen(rect, rect);
			SDL_RenderFillRectF(GetRenderer(), &rect);
		}
	}
}

void Application::RenderDebug()
{
	RenderCommon();

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
			SDL_RenderDrawLineF(GetRenderer(), p1.x, p1.y, p2.x, p2.y);
		}
	};

	auto DrawNode = [this](Node &node) -> void
	{
		SDL_FRect rect = { float(node.pos.x), float(node.pos.y), 1, 1 };

		vPort.RectToScreen(rect, rect);
		SDL_RenderFillRectF(GetRenderer(), &rect);
	};

	SDL_SetRenderDrawColor(GetRenderer(), 0xFF, 0, 0, 0xFF);
	bt::Node<Cell>::SetDefaultPreorder();

	for (auto& btNode : *gen.root)
	{
		if (btNode.m_left != nullptr || btNode.m_right != nullptr) continue;
		SDL_FRect rect = ToFRect(btNode.space);

		vPort.RectToScreen(rect, rect);
		SDL_RenderDrawRectF(GetRenderer(), &rect);
	}

	for (Room &room : gen.rooms)
	{
		SDL_SetRenderDrawColor(GetRenderer(), 0, 0xAA, 0xAA, 0xFF);
		for (Rect &rect : room.rects)
		{
			SDL_FRect sdlRect = ToFRect(rect);
			vPort.RectToScreen(sdlRect, sdlRect);
			SDL_RenderDrawRectF(GetRenderer(), &sdlRect);
		}

		SDL_SetRenderDrawColor(GetRenderer(), 0x50, 0x50, 0x50, 0xFF);
		DrawLinks(room);
	}

	SDL_SetRenderDrawColor(GetRenderer(), 0x50, 0x50, 0x50, 0xFF);
	for (auto &[pair, node] : gen.posXNodes) DrawLinks(node);

	SDL_SetRenderDrawColor(GetRenderer(), 0, 0xC0, 0, 0xFF);
	for (Room &room : gen.rooms) DrawNode(room);

	for (auto &[pair, node] : gen.posXNodes) DrawNode(node);
}

void Application::RenderCommon()
{
	SDL_Renderer *const renderer = GetRenderer();

	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	const float scale = vPort.GetScale();
	if (scale >= gGridMinimumScale)
	{
		SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 0xFF);

		auto Limit = [scale](float& var, float min, float max) -> void
		{
			if (var < min) var += std::floorf((min - var) / scale) * scale;
			else if (var > max) var -= std::floorf((var - max) / scale) * scale;
		};

		SDL_FPoint p1, p2;

		vPort.ToScreen(0.0f, 0.0f, p1.x, p1.y);
		vPort.ToScreen(static_cast<float>(gInput.xSize), static_cast<float>(gInput.ySize), p2.x, p2.y);

		const float xMax = static_cast<float>(GetSize().x);
		const float yMax = static_cast<float>(GetSize().y);

		Limit(p1.x, 0, xMax);
		Limit(p1.y, 0, yMax);
		Limit(p2.x, 0, xMax);
		Limit(p2.y, 0, yMax);

		for (float x = p1.x; x <= p2.x; x += scale)
			SDL_RenderDrawLineF(renderer, x, p1.y, x, p2.y);

		for (float y = p1.y; y <= p2.y; y += scale)
			SDL_RenderDrawLineF(renderer, p1.x, y, p2.x, y);
	}
}

void Application::ApplyFactor()
{
	const float invFactor = 1 / factor;

	gInput.xSize = int(GetSize().x * invFactor);
	gInput.ySize = int(GetSize().y * invFactor);

	vPort.SetDefaultScale(factor);
	vPort.Reset();

	lastFactor = factor;
}

void Application::LoadDefaults()
{
	gInput.generateFewerPaths = true;

	gInput.randAreaDens = gDefRandAreaDens;
	gInput.randAreaProb = gDefRandAreaProb;
	gInput.randAreaDepth = gDefRandAreaDepth;

	gInput.xSize = GetSize().x;
	gInput.ySize = GetSize().y;
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

	static uint seed = 0;

	try
	{
		if (mode != GenMode::DEBUG_MODE)
		{
			#ifdef INCREMENTAL_SEED
			seed += mode == GenMode::NEW_SEED;
			#else
			static std::random_device rd;
			if (mode == GenMode::NEW_SEED) seed = rd();
			#endif

			gen.Generate(&gInput, &gOutput, seed);
		}
		else gen.GenerateDebug(&gInput, &gOutput, seed, &RenderDebugHelper, this);
	}
	catch (const std::exception &error) { std::cerr << error.what() << "\n"; }
}