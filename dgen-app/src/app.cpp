// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "app.hpp"
#include "widgets/menu.hpp"
#include "widgets/info.hpp"
#include "widgets/help.hpp"
#include "widgets/input.hpp"
#include "widgets/warning.hpp"

#include <cmath>
#include <stdexcept>

Application* Widget::s_app = nullptr;
Widget* Widget::s_active = nullptr;

void Application::Draw()
{
	SDL_Renderer* const renderer = GetRenderer();

	SDL_SetRenderTarget(renderer, nullptr);
	SDL_RenderCopy(renderer, m_renderOutput, nullptr, nullptr);

	for (Widget* crr = m_widgetList; crr != nullptr; crr = crr->m_next)
		crr->Draw();

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

		SDL_FPoint p1{};
		m_viewport.ToScreen(0.0f, 0.0f, p1.x, p1.y);

		SDL_FPoint p2{};
		m_viewport.ToScreen(static_cast<float>(m_input.m_width), static_cast<float>(m_input.m_height), p2.x, p2.y);

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
		for (auto& node : m_generator.m_rootNode->Preorder())
		{
			if (node.m_left != nullptr || node.m_right != nullptr)
				continue;

			if (node.m_flags & (1 << dg::impl::Cell::Flag::SPARSE_AREA))
				SDL_SetRenderDrawColor(renderer, 0x50, 0x40, 0x40, 0xFF);
			else
				SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);

			SDL_FRect rect;
			m_viewport.RectToScreen(node.m_space, rect);
			SDL_RenderDrawRectF(renderer, &rect);
		}

		for (dg::impl::Room& room : m_generator.m_rooms)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0xAA, 0xAA, 0xFF);
			for (size_t i = room.m_rectBegin; i < room.m_rectEnd; i++)
			{
				const dg::Rect& rect = m_output.m_rooms[i];
				SDL_FRect sdlRect;

				m_viewport.RectToScreen(rect, sdlRect);
				SDL_RenderDrawRectF(renderer, &sdlRect);
			}

			SDL_SetRenderDrawColor(renderer, 0x80, 0, 0x80, 0xFF);
			for (int i = 0; i < 4; i++)
			{
				if (room.m_links[i] == &dg::impl::Vertex::s_sentinel)
					continue;

				SDL_FPoint p1{ room.m_entrances[i].x + 0.5f, room.m_entrances[i].y + 0.5f };
				SDL_FPoint p2{ room.m_links[i]->m_pos.x + 0.5f, room.m_links[i]->m_pos.y + 0.5f };

				m_viewport.ToScreen(p1.x, p1.y, p1.x, p1.y);
				m_viewport.ToScreen(p2.x, p2.y, p2.x, p2.y);

				SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}

		SDL_SetRenderDrawColor(renderer, 0x50, 0x50, 0x50, 0xFF);
		for (dg::impl::Vertex& vertex : m_generator.m_vertices)
		{
			SDL_FPoint p1 = { static_cast<float>(vertex.m_pos.x + 0.5f), static_cast<float>(vertex.m_pos.y + 0.5f) };
			m_viewport.ToScreen(p1.x, p1.y, p1.x, p1.y);

			const int end = (m_input.m_seed & 0b10) + 2;
			for (int i = m_input.m_seed & 0b10; i < end; i++)
			{
				dg::impl::Vertex* const vertex2 = vertex.m_links[i];
				if (vertex2 == &dg::impl::Vertex::s_sentinel || vertex2->ToRoom() != nullptr)
					continue;

				SDL_FPoint p2 = { static_cast<float>(vertex2->m_pos.x + 0.5f), static_cast<float>(vertex2->m_pos.y + 0.5f) };
				m_viewport.ToScreen(p2.x, p2.y, p2.x, p2.y);
				SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}

		for (dg::impl::Room& room : m_generator.m_rooms)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0xC0, 0, 0xFF);
			SDL_FRect rect = { static_cast<float>(room.m_pos.x), static_cast<float>(room.m_pos.y), 1, 1 };

			m_viewport.RectToScreen(rect, rect);
			SDL_RenderFillRectF(renderer, &rect);

			SDL_SetRenderDrawColor(renderer, 0x80, 0, 0x80, 0xFF);
			for (dg::Point& point : room.m_entrances)
			{
				rect.x = static_cast<float>(point.x);
				rect.y = static_cast<float>(point.y);
				rect.w = 1.0f; rect.h = 1.0f;

				m_viewport.RectToScreen(rect, rect);
				SDL_RenderFillRectF(renderer, &rect);
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0xC0, 0, 0xFF);
		for (dg::impl::Vertex& vertex : m_generator.m_vertices)
		{
			bool notEmpty = false;
			for (const auto& link : vertex.m_links)
				notEmpty |= link != &dg::impl::Vertex::s_sentinel;

			if (notEmpty)
			{
				SDL_FRect rect = { static_cast<float>(vertex.m_pos.x), static_cast<float>(vertex.m_pos.y), 1, 1 };
				m_viewport.RectToScreen(rect, rect);
				SDL_RenderFillRectF(renderer, &rect);
			}
		}
	}
	else
	{
		if (m_visRooms)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0xAA, 0xAA, 0xFF);
			for (dg::Rect& room : m_output.m_rooms)
			{
				SDL_FRect rect;
				m_viewport.RectToScreen(room, rect);
				SDL_RenderDrawRectF(renderer, &rect);
			}
		}

		if (m_visPaths)
		{
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			for (auto& [origin, offset] : m_output.m_paths)
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
			SDL_SetRenderDrawColor(renderer, 0x80, 0, 0x80, 0xFF);
			for (dg::Point& entrance : m_output.m_entrances)
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
	for (Widget* crr = m_widgetList; crr != nullptr; crr = crr->m_next)
	{
		Animator* const animator = crr->m_animator;
		if (animator != nullptr && animator->IsPlaying())
		{
			animator->Update();
			Schedule(Task::DRAW);
		}
	}

	SDL_Event sdlEvent{};
	bool pending = (m_task != Task::IDLE) ? SDL_PollEvent(&sdlEvent) : SDL_WaitEvent(&sdlEvent);

	while (pending)
	{
		if (sdlEvent.type == SDL_QUIT)
			return false;

		if (Widget::s_active != nullptr)
		{
			Widget::s_active->HandleEvent(sdlEvent);
			pending = SDL_PollEvent(&sdlEvent);
			continue;
		}

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

		for (Widget* crr = m_widgetList; crr != nullptr; crr = crr->m_next)
			crr->HandleEvent(sdlEvent);

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

	m_input.m_width = static_cast<int>(GetWidth() / m_factor);
	m_input.m_height = static_cast<int>(GetHeight() / m_factor);

	try
	{
		if (m_seedMode == SeedMode::INCREMENT) m_input.m_seed++;
		else if (m_seedMode == SeedMode::RANDOMIZE) m_input.m_seed = m_randomDevice();

		m_generator.Generate(&m_input, &m_output);

		const std::string seed = std::to_string(m_input.m_seed);
		const std::string rooms = std::to_string(m_output.m_rooms.size());
		const std::string paths = std::to_string(m_output.m_paths.size());
		const std::string entrances = std::to_string(m_output.m_entrances.size());

		AccessWidget<Info>().Set("seed: " + seed + ", rooms: " + rooms + ", paths: " + paths + ", entrances: " + entrances);

		if (Warning* warning = GetWidget<Warning>(); warning != nullptr)
			warning->Set("");
	}
	catch (const std::exception& error)
	{
		AccessWidget<Info>().Set("seed: " + std::to_string(m_input.m_seed) + ", rooms: - , paths: - , entrances: - ");
		AccessWidget<Warning>().Set(error.what());
	}
}

void Application::LoadDefaults()
{
	m_factor = g_factor;
	m_visRooms = g_visRooms;
	m_visPaths = g_visPaths;
	m_visEntrances = g_visEntrances;

	m_input.m_seed = 0;
	m_input.m_minDepth = g_minDepth;
	m_input.m_maxDepth = g_maxDepth;
	m_input.m_minRoomSize = g_minRoomSize;
	m_input.m_maxRoomSize = g_maxRoomSize;
	m_input.m_pathCostFactor = g_pathCostFactor;
	m_input.m_extraPathCount = g_extraPathCount;
	m_input.m_extraPathDepth = g_extraPathDepth;
	m_input.m_sparseAreaDens = g_sparseAreaDens;
	m_input.m_sparseAreaProb = g_sparseAreaProb;
	m_input.m_sparseAreaDepth = g_sparseAreaDepth;
	m_input.m_doubleRoomProb = g_doubleRoomProb;
	m_input.m_heuristicFactor = g_heuristicFactor;
	m_input.m_generateFewerPaths = g_generateFewerPaths;
	m_input.m_spaceInterdistance = g_spaceInterdistance;
	m_input.m_spaceSizeRandomness = g_spaceSizeRandomness;

	if (Menu* menu = GetWidget<Menu>(); menu != nullptr)
		menu->ScheduleRendering();
}

void Application::SetupWidgets()
{
	static bool s_displayHelp = true;
	if (s_displayHelp)
	{
		s_displayHelp = false;
		Widget::s_active = &AccessWidget<Help>();
	}

	Menu& menu = AccessWidget<Menu>();

	menu.Add<FactorMod>("Size factor", m_factor);
	menu.Add<IntMod>("Minimum depth", m_input.m_minDepth);
	menu.Add<IntMod>("Maximum depth", m_input.m_maxDepth);
	menu.Add<IntMod>("Space interdistance", m_input.m_spaceInterdistance);
	menu.Add<PercentMod>("Space randomness", m_input.m_spaceSizeRandomness);
	menu.Add<IntMod>("Sparse area depth", m_input.m_sparseAreaDepth);
	menu.Add<PercentMod>("Sparse area density", m_input.m_sparseAreaDens);
	menu.Add<PercentMod>("Sparse area probability", m_input.m_sparseAreaProb);
	menu.Add<PercentMod>("Minimum room size", m_input.m_minRoomSize);
	menu.Add<PercentMod>("Maximum room size", m_input.m_maxRoomSize);
	menu.Add<PercentMod>("Double room probability", m_input.m_doubleRoomProb);
	menu.Add<PercentMod>("Heuristic", m_input.m_heuristicFactor);
	menu.Add<PercentMod>("Path cost factor", m_input.m_pathCostFactor);
	menu.Add<IntMod>("Extra path count", m_input.m_extraPathCount);
	menu.Add<IntMod>("Extra path depth", m_input.m_extraPathDepth);
	menu.Add<BoolMod>("Rooms visibility", m_visRooms);
	menu.Add<BoolMod>("Paths visibility", m_visPaths);
	menu.Add<BoolMod>("Entrances visibility", m_visEntrances);
}

void Application::RenderWidgets()
{
	for (Widget* crr = m_widgetList; crr != nullptr; crr = crr->m_next)
	{
		if (crr->m_render)
		{
			crr->Render();
			crr->m_render = false;
		}
	}
}

void Application::Init(bool full)
{
	Quit(full);

	if (full)
		LoadFont(0, 16, g_fontData, g_fontDataSize);

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