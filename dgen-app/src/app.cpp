#include "app.hpp"
#include "widgets/menu.hpp"
#include "widgets/warning.hpp"

#include <cmath>
#include <stdexcept>

SDL_FPoint ToFPoint(const Point& point) { return { static_cast<float>(point.x), static_cast<float>(point.y) }; }
SDL_FRect ToFRect(const Rect& rect) { return { static_cast<float>(rect.x), static_cast<float>(rect.y), static_cast<float>(rect.w), static_cast<float>(rect.h) }; }

Application* Widget::appPointer = nullptr;

void Application::Draw()
{
	SDL_Renderer* const renderer = GetRenderer();

	SDL_SetRenderTarget(renderer, nullptr);
	SDL_RenderCopy(renderer, m_renderOutput, nullptr, nullptr);

	for (Widget* crr = m_widgetList; crr != nullptr; crr = crr -> m_next)
		crr -> Draw();

	SDL_RenderPresent(renderer);
}

void Application::Render()
{
	SDL_Renderer* const renderer = GetRenderer();

	SDL_SetRenderTarget(renderer, m_renderOutput);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	const float scale = m_viewport.GetScale();
	if (scale >= g_gridThresholdScale)
	{
		auto Limit = [scale](float& var, float min, float max) -> void
		{
			if (var < min) var += std::floor((min - var) / scale) * scale;
			else if (var > max) var -= std::floor((var - max) / scale) * scale;
		};

		SDL_FPoint p1 = {};
		m_viewport.ToScreen(0.0f, 0.0f, p1.x, p1.y);

		SDL_FPoint p2 = {};
		m_viewport.ToScreen(static_cast<float>(m_input.width), static_cast<float>(m_input.height), p2.x, p2.y);

		const float xMax = static_cast<float>(GetWidth());
		const float yMax = static_cast<float>(GetHeight());

		Limit(p1.x, 0, xMax);
		Limit(p1.y, 0, yMax);
		Limit(p2.x, 0, xMax);
		Limit(p2.y, 0, yMax);

		SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 0xFF);

		for (float x = p1.x; x <= p2.x; x += scale)
			SDL_RenderDrawLineF(renderer, x, p1.y, x, p2.y);

		for (float y = p1.y; y <= p2.y; y += scale)
			SDL_RenderDrawLineF(renderer, p1.x, y, p2.x, y);
	}

	if (m_debugView)
	{
		auto DrawLinks = [this, renderer](Node& node) -> void
		{
			SDL_FPoint p1 = { static_cast<float>(node.pos.x + 0.5f), static_cast<float>(node.pos.y + 0.5f) };
			m_viewport.ToScreen(p1.x, p1.y, p1.x, p1.y);

			const int end = (m_input.seed & 0b10) + 2;
			for (int i = m_input.seed & 0b10; i < end; i++)
			{
				Node* const node2 = node.links[i];
				if (node2 == nullptr) continue;

				SDL_FPoint p2 = { static_cast<float>(node2 -> pos.x + 0.5f), static_cast<float>(node2 -> pos.y + 0.5f) };
				m_viewport.ToScreen(p2.x, p2.y, p2.x, p2.y);
				SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
			}
		};

		auto DrawNode = [this, renderer](Node& node) -> void
		{
			SDL_FRect rect = { static_cast<float>(node.pos.x), static_cast<float>(node.pos.y), 1, 1 };
			m_viewport.RectToScreen(rect, rect);
			SDL_RenderFillRectF(renderer, &rect);
		};

		SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
		bt::Node<Cell>::defaultTraversal = bt::Traversal::PREORDER;

		for (auto& btNode : *m_generator.root)
		{
			if (btNode.m_left != nullptr || btNode.m_right != nullptr)
				continue;

			SDL_FRect rect = ToFRect(btNode.space);
			m_viewport.RectToScreen(rect, rect);
			SDL_RenderDrawRectF(renderer, &rect);
		}

		for (Room& room : m_generator.rooms)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0xAA, 0xAA, 0xFF);
			for (Rect& rect : room.rects)
			{
				SDL_FRect sdlRect = ToFRect(rect);
				m_viewport.RectToScreen(sdlRect, sdlRect);
				SDL_RenderDrawRectF(renderer, &sdlRect);
			}

			SDL_SetRenderDrawColor(renderer, 0x50, 0x50, 0x50, 0xFF);
			DrawLinks(room);
		}

		SDL_SetRenderDrawColor(renderer, 0x50, 0x50, 0x50, 0xFF);
		for (auto& [pos, node] : m_generator.posXNodes) DrawLinks(node);

		SDL_SetRenderDrawColor(renderer, 0, 0xC0, 0, 0xFF);
		for (Room& room : m_generator.rooms) DrawNode(room);
		for (auto& [pos, node] : m_generator.posXNodes) DrawNode(node);
	}
	else
	{
		if (m_visRooms)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0xAA, 0xAA, 0xFF);
			for (Rect& room : m_output.rooms)
			{
				SDL_FRect rect = ToFRect(room);
				m_viewport.RectToScreen(rect, rect);
				SDL_RenderDrawRectF(renderer, &rect);
			}
		}

		if (m_visPaths)
		{
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			for (auto& [origin, offset] : m_output.paths)
			{
				SDL_FPoint p1 = { origin.x + 0.5f, origin.y + 0.5f };
				SDL_FPoint p2 = { offset.x + p1.x, offset.y + p1.y };

				m_viewport.ToScreen(p1.x, p1.y, p1.x, p1.y);
				m_viewport.ToScreen(p2.x, p2.y, p2.x, p2.y);

				SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}

		if (m_visEntrances)
		{
			SDL_SetRenderDrawColor(renderer, 0xFF, 0x60, 0, 0xFF);
			for (Point& entrance : m_output.entrances)
			{
				SDL_FRect rect = { static_cast<float>(entrance.x), static_cast<float>(entrance.y), 1, 1 };
				m_viewport.RectToScreen(rect, rect);
				SDL_RenderFillRectF(renderer, &rect);
			}
		}
	}
}

bool Application::Update()
{
	for (Widget* crr = m_widgetList; crr != nullptr; crr = crr -> m_next)
	{
		Animator* const animator = crr -> m_animator;
		if (animator != nullptr && animator -> IsPlaying())
		{
			animator -> Update();
			Schedule(Task::DRAW);
		}
	}

	SDL_Event sdlEvent = {};
	bool pending = (m_task != Task::IDLE) ? SDL_PollEvent(&sdlEvent) : SDL_WaitEvent(&sdlEvent);

	while (pending)
	{
		if (sdlEvent.type == SDL_QUIT)
			return false;

		if (sdlEvent.type == SDL_KEYDOWN)
		{
			switch (sdlEvent.key.keysym.sym)
			{
			case SDLK_g:
				ScheduleGeneration(SeedMode::RANDOMIZE);
				break;

			case SDLK_n:
				ScheduleGeneration(SeedMode::INCREMENT);
				break;

			case SDLK_r:
				ScheduleGeneration(SeedMode::KEEP);
				LoadDefaults();
				break;

			case SDLK_d:
				Schedule(Task::RENDER);
				m_debugView = !m_debugView;
				break;

			case SDLK_TAB:
				Schedule(Task::RENDER);
				m_viewport.Reset();
				break;

			case SDLK_F11:
				ScheduleGeneration(SeedMode::KEEP);
				m_fullscreen = !m_fullscreen;
				Init(false);
				break;

			case SDLK_ESCAPE:
				return false;
			}
		}

		if (m_viewport.Update(sdlEvent))
			Schedule(Task::RENDER);

		for (Widget* crr = m_widgetList; crr != nullptr; crr = crr -> m_next)
			crr -> HandleEvent(sdlEvent);

		pending = SDL_PollEvent(&sdlEvent);
	}

	switch (m_task)
	{
	case Task::GENERATE:
		Generate();
		[[fallthrough]];

	case Task::RENDER:
		Render();
		[[fallthrough]];

	case Task::RENDER_WIDGETS:
		RenderWidgets();
		[[fallthrough]];

	case Task::DRAW:
		Draw();
		[[fallthrough]];

	default:
		m_task = Task::IDLE;
	}

	return true;
}

void Application::Generate()
{
	m_viewport.SetDefaultScale(m_factor);

	m_input.width = static_cast<int>(GetWidth() / m_factor);
	m_input.height = static_cast<int>(GetHeight() / m_factor);

	try
	{
		if (m_seedMode == SeedMode::INCREMENT) m_input.seed++;
		else if (m_seedMode == SeedMode::RANDOMIZE) m_input.seed = m_randomDevice();

		m_generator.Generate(&m_input, &m_output);

		if (Warning* warning = GetWidget<Warning>(); warning != nullptr)
			warning -> Set("");
	}
	catch (const std::exception& error) { AccessWidget<Warning>().Set(error.what()); }
}

void Application::LoadDefaults()
{
	m_factor = g_factor;
	m_visRooms = g_visRooms;
	m_visPaths = g_visPaths;
	m_visEntrances = g_visEntrances;

	m_input.seed = 0;
	m_input.minDepth = g_minDepth;
	m_input.maxDepth = g_maxDepth;
	m_input.maxRoomSize = g_maxRoomSize;
	m_input.minRoomSize = g_minRoomSize;
	m_input.randAreaDens = g_randAreaDens;
	m_input.randAreaProb = g_randAreaProb;
	m_input.randAreaDepth = g_randAreaDepth;
	m_input.doubleRoomProb = g_doubleRoomProb;
	m_input.heuristicFactor = g_heuristicFactor;
	m_input.generateFewerPaths = g_generateFewerPaths;
	m_input.spaceInterdistance = g_spaceInterdistance;
	m_input.spaceSizeRandomness = g_spaceSizeRandomness;

	if (Menu* menu = GetWidget<Menu>(); menu != nullptr)
		menu -> ScheduleRendering();
}

void Application::SetupWidgets()
{
	Menu& menu = AccessWidget<Menu>();

	menu.Add<FactorMod>("Size factor", m_factor);
	menu.Add<IntMod>("Minimum depth", m_input.minDepth);
	menu.Add<IntMod>("Maximum depth", m_input.maxDepth);
	menu.Add<PercentMod>("Heuristic", m_input.heuristicFactor);
	menu.Add<PercentMod>("Minimum room size", m_input.minRoomSize);
	menu.Add<PercentMod>("Maximum room size", m_input.maxRoomSize);
	menu.Add<PercentMod>("Space randomness", m_input.spaceSizeRandomness);
	menu.Add<PercentMod>("Double room probability", m_input.doubleRoomProb);
	menu.Add<IntMod>("Space interdistance", m_input.spaceInterdistance);
	menu.Add<IntMod>("Random area depth", m_input.randAreaDepth);
	menu.Add<PercentMod>("Random area density", m_input.randAreaDens);
	menu.Add<PercentMod>("Random area probability", m_input.randAreaProb);
	menu.Add<BoolMod>("Rooms visibility", m_visRooms);
	menu.Add<BoolMod>("Paths visibility", m_visPaths);
	menu.Add<BoolMod>("Entrances visibility", m_visEntrances);
}

void Application::RenderWidgets()
{
	for (Widget* crr = m_widgetList; crr != nullptr; crr = crr -> m_next)
	{
		if (crr -> m_render)
		{
			crr -> Render();
			crr -> m_render = false;
		}
	}
}

void Application::Init(bool full)
{
	Quit(full);

	if (full)
	{
		int index = 0;
		for (const auto& [size, path] : g_fonts)
			LoadFont(index++, size, path);
	}

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	if (m_fullscreen) CreateWindow(g_title, dm.w, dm.h, SDL_WINDOW_HIDDEN | SDL_WINDOW_FULLSCREEN);
	else CreateWindow(g_title, dm.w - 30, dm.h - 100, SDL_WINDOW_HIDDEN);

	m_renderOutput = CreateTexture(GetWidth(), GetHeight(), false);

	SetupWidgets();
	SDL_ShowWindow(GetWindow());
}

void Application::Quit(bool full)
{
	if (m_widgetList != nullptr)
	{
		delete m_widgetList;
		m_widgetList = nullptr;
	}

	if (m_renderOutput != nullptr)
	{
		SDL_DestroyTexture(m_renderOutput);
		m_renderOutput = nullptr;
	}

	if (full) ResetAppManager();
}

void Application::Run()
{
	Init(true);
	while (Update()) {}
}