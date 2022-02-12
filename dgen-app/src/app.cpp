#include <iostream>
#include <cmath>
#include <random>
#include <stdexcept>
#include "app.hpp"

inline SDL_FPoint ToFPoint(const Point &point) { return { float(point.x), float(point.y) }; }
inline SDL_FRect ToFRect(const Rect &rect) { return { float(rect.x), float(rect.y), float(rect.w), float(rect.h) }; }

Application::Application() : plus(false), debug(false), fullscreen(false), factor(1), texture(nullptr), seed(0)
{
	for (int i = 0; i < sizeof(g_fonts) / sizeof(*g_fonts); i++)
	{
		const auto& font = g_fonts[i];
		LoadFont(i, font.first, font.second);
	}

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

	overlay -> AddMod(BoolMod("Rooms visibility", roomsVisibility));
	overlay -> AddMod(BoolMod("Paths visibility", pathsVisibility));
	overlay -> AddMod(BoolMod("Entrances visibility", entrancesVisibility));
}

Application::~Application()
{
	delete overlay;
	if (texture != nullptr) SDL_DestroyTexture(texture);
}

void Application::Run()
{
	InitWindow();
	LoadDefaults();

	Generate(GenMode::REFRESH);
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

void Application::Render()
{
	SDL_Renderer *const renderer = GetRenderer();

	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	const float scale = vPort.GetScale();
	if (scale >= g_gridThresholdScale)
	{
		SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 0xFF);

		auto Limit = [scale](float& var, float min, float max) -> void
		{
			if (var < min) var += std::floor((min - var) / scale) * scale;
			else if (var > max) var -= std::floor((var - max) / scale) * scale;
		};

		SDL_FPoint p1, p2;

		vPort.ToScreen(0.0f, 0.0f, p1.x, p1.y);
		vPort.ToScreen(static_cast<float>(gInput.width), static_cast<float>(gInput.height), p2.x, p2.y);

		const float xMax = static_cast<float>(GetWidth());
		const float yMax = static_cast<float>(GetHeight());

		Limit(p1.x, 0, xMax);
		Limit(p1.y, 0, yMax);
		Limit(p2.x, 0, xMax);
		Limit(p2.y, 0, yMax);

		for (float x = p1.x; x <= p2.x; x += scale)
			SDL_RenderDrawLineF(renderer, x, p1.y, x, p2.y);

		for (float y = p1.y; y <= p2.y; y += scale)
			SDL_RenderDrawLineF(renderer, p1.x, y, p2.x, y);
	}

	if (debug)
	{
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

		SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
		bt::Node<Cell>::SetDefaultPreorder();

		for (auto& btNode : *gen.root)
		{
			if (btNode.m_left != nullptr || btNode.m_right != nullptr) continue;
			SDL_FRect rect = ToFRect(btNode.space);

			vPort.RectToScreen(rect, rect);
			SDL_RenderDrawRectF(renderer, &rect);
		}

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
	else
	{
		if (roomsVisibility)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0xAA, 0xAA, 0xFF);
			for (Rect &room : gOutput.rooms)
			{
				SDL_FRect rect = ToFRect(room);
				vPort.RectToScreen(rect, rect);
				SDL_RenderDrawRectF(renderer, &rect);
			}
		}

		if (pathsVisibility)
		{
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			for (std::pair<Point, Vec> &path : gOutput.paths)
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

		if (entrancesVisibility)
		{
			SDL_SetRenderDrawColor(renderer, 0xFF, 0x60, 0, 0xFF);
			for (Point &entrance : gOutput.entrances)
			{
				SDL_FPoint point = ToFPoint(entrance);
				SDL_FRect rect = { point.x, point.y, 1, 1 };

				vPort.RectToScreen(rect, rect);
				SDL_RenderFillRectF(renderer, &rect);
			}
		}
	}
}

bool Application::Update()
{
	SDL_Event sdlEvent = {};

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

		case SDLK_F11:
			render = true;
			overlay -> refresh = true;
			fullscreen = !fullscreen;

			InitWindow();
			Generate(GenMode::REFRESH);
			break;

		case SDLK_TAB:
			render = true;
			vPort.Reset();
			break;

		case SDLK_g:
			render = true;
			Generate(GenMode::RAND_SEED);
			break;

		case SDLK_n:
			render = true;
			Generate(GenMode::NEXT_SEED);
			break;

		case SDLK_r:
			render = true;
			overlay -> refresh = true;

			LoadDefaults();
			Generate(GenMode::REFRESH);
			break;

		case SDLK_d:
			render = true;
			debug = !debug;
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
				Generate(GenMode::REFRESH);
			}
			break;

		case SDLK_LEFT:
		case SDLK_MINUS:
			if (overlay -> ChangeSelected(true))
			{
				plus = false;
				render = true;
				Generate(GenMode::REFRESH);
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

void Application::InitWindow()
{
	overlay -> DestroyResources();
	if (texture != nullptr) SDL_DestroyTexture(texture);

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	if (fullscreen) CreateWindow(g_title, dm.w, dm.h, SDL_WINDOW_HIDDEN | SDL_WINDOW_FULLSCREEN);
	else CreateWindow(g_title, dm.w - 30, dm.h - 100, SDL_WINDOW_HIDDEN);

	texture = SDL_CreateTexture(GetRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GetWidth(), GetHeight());
	overlay -> InitResources();

	SDL_ShowWindow(GetWindow());
}

void Application::LoadDefaults()
{
	factor = g_factor;

	gInput.minDepth = g_minDepth;
	gInput.maxDepth = g_maxDepth;
	gInput.maxRoomSize = g_maxRoomSize;
	gInput.minRoomSize = g_minRoomSize;
	gInput.randAreaDens = g_randAreaDens;
	gInput.randAreaProb = g_randAreaProb;
	gInput.randAreaDepth = g_randAreaDepth;
	gInput.doubleRoomProb = g_doubleRoomProb;
	gInput.heuristicFactor = g_heuristicFactor;
	gInput.generateFewerPaths = g_generateFewerPaths;
	gInput.spaceSizeRandomness = g_spaceSizeRandomness;
	gInput.additionalConnections = g_additionalConnections;

	roomsVisibility = g_roomsVisibility;
	pathsVisibility = g_pathsVisibility;
	entrancesVisibility = g_entrancesVisibility;
}

void Application::Generate(GenMode mode)
{
	vPort.SetDefaultScale(factor);

	gInput.width = int(GetWidth() / factor);
	gInput.height = int(GetHeight() / factor);

	if (gInput.randAreaDepth > gInput.maxDepth)
		gInput.randAreaDepth = gInput.maxDepth;

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

	try
	{
		if (mode == GenMode::NEXT_SEED) seed++;
		else if (mode == GenMode::RAND_SEED) seed = rd();

		gen.Generate(&gInput, &gOutput, seed);
	}
	catch (const std::exception& error) { std::cerr << error.what() << "\n"; }
}