// SPDX-FileCopyrightText: Copyright (c) 2026 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "pch.hpp"
#include "app.hpp"

void Application::load_defaults()
{
	m_factor = 1.0f;
	m_visRooms = true;
	m_visPaths = true;
	m_visEntrances = false;

	m_input.m_seed = 0;
	m_input.m_minDepth = 7;
	m_input.m_maxDepth = 9;
	m_input.m_minRoomSize = 0.5f;
	m_input.m_maxRoomSize = 0.8f;
	m_input.m_pathCostFactor = 0.1f;
	m_input.m_extraPathCount = 5;
	m_input.m_extraPathDepth = 3;
	m_input.m_sparseAreaDens = 0.1f;
	m_input.m_sparseAreaProb = 0.3f;
	m_input.m_sparseAreaDepth = 1;
	m_input.m_doubleRoomProb = 0.35f;
	m_input.m_heuristicFactor = 0.2f;
	m_input.m_generateFewerPaths = true;
	m_input.m_spaceInterdistance = 1;
	m_input.m_spaceSizeRandomness = 0.25f;
}

void Application::init()
{
	cleanup();

	SDL_DisplayID display = SDL_GetPrimaryDisplay();
	if (display == 0)
		throw std::runtime_error(SDL_GetError());

	const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display);
	if (mode == nullptr)
		throw std::runtime_error(SDL_GetError());

	SDL_Rect usableBounds;
	if (!SDL_GetDisplayUsableBounds(display, &usableBounds))
		throw std::runtime_error(SDL_GetError());

	m_width = static_cast<int>(usableBounds.w * 0.95);
	m_height = static_cast<int>(usableBounds.h * 0.9);

	if (!SDL_CreateWindowAndRenderer("Dungeon Generator", m_width, m_height, 0, &m_window, &m_renderer))
		throw std::runtime_error(SDL_GetError());

	m_context = ImGui::CreateContext();
	m_io = &ImGui::GetIO();

	ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
	ImGui_ImplSDLRenderer3_Init(m_renderer);

	m_dungeon = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_width, m_height);
	m_frameDelay = std::chrono::duration_cast<clk_t::duration>(std::chrono::duration<double>(1.0 / mode->refresh_rate));
	m_nextRefresh = clk_t::now() + m_frameDelay;

	load_defaults();
}

void Application::cleanup()
{
	if (m_dungeon)
		SDL_DestroyTexture(std::exchange(m_dungeon, nullptr));

	if (m_renderer)
	{
		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		SDL_DestroyRenderer(std::exchange(m_renderer, nullptr));
	}

	if (m_window)
		SDL_DestroyWindow(std::exchange(m_window, nullptr));
}

void Application::handle_events()
{
	SDL_Event e{};
	while (SDL_PollEvent(&e))
	{
		ImGui_ImplSDL3_ProcessEvent(&e);
		if (e.type == SDL_EVENT_QUIT || (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
			m_running = false;

		if (!m_io->WantCaptureMouse && m_viewport.update(e))
			m_renderReq = true;
	}

	if (m_generateReq)
		generate();

	if (m_renderReq || m_generateReq)
		render();

	m_renderReq = false;
	m_generateReq = false;
}

void Application::generate()
{
	m_input.m_width = static_cast<int>(m_width / m_factor);
	m_input.m_height = static_cast<int>(m_height / m_factor);

	try
	{
		m_generator.Generate(&m_input, &m_output);
		m_error.clear();
	}
	catch (const std::exception& error)
	{
		m_output = {};
		m_error = error.what();
	}
}

void Application::render()
{
	SDL_SetRenderTarget(m_renderer, m_dungeon);
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(m_renderer);

	const float scale = m_viewport.get_scale();
	if (scale >= 8.0f)
	{
		auto limit = [scale](float& var, float min, float max) -> void
		{
			if (var < min)
				var += std::floor((min - var) / scale) * scale;
			else if (var > max)
				var -= std::floor((var - max) / scale) * scale;
		};

		SDL_FPoint p1 = m_viewport.to_screen(SDL_FPoint{ 0, 0 });
		SDL_FPoint p2 = m_viewport.to_screen(SDL_FPoint{ static_cast<float>(m_input.m_width), static_cast<float>(m_input.m_height) });

		limit(p1.x, 0.0f, m_width);
		limit(p1.y, 0.0f, m_height);
		limit(p2.x, 0.0f, m_width);
		limit(p2.y, 0.0f, m_height);

		SDL_SetRenderDrawColor(m_renderer, 0x16, 0x16, 0x16, 0xFF);

		for (float x = p1.x; x <= p2.x; x += scale)
			SDL_RenderLine(m_renderer, x, p1.y, x, p2.y);

		for (float y = p1.y; y <= p2.y; y += scale)
			SDL_RenderLine(m_renderer, p1.x, y, p2.x, y);
	}

	if (m_debugView)
	{
		for (auto& node : m_generator.m_rootNode->Preorder())
		{
			if (node.m_left != nullptr || node.m_right != nullptr)
				continue;

			if (node.m_flags & (1 << dg::impl::Cell::Flag::SPARSE_AREA))
				SDL_SetRenderDrawColor(m_renderer, 0x50, 0x40, 0x40, 0xFF);
			else
				SDL_SetRenderDrawColor(m_renderer, 0xFF, 0, 0, 0xFF);

			SDL_FRect rect = m_viewport.to_screen<dg::Rect, SDL_FRect>(node.m_space);
			SDL_RenderRect(m_renderer, &rect);
		}

		for (dg::impl::Room& room : m_generator.m_rooms)
		{
			SDL_SetRenderDrawColor(m_renderer, 0, 0xAA, 0xAA, 0xFF);
			for (size_t i = room.m_rectBegin; i < room.m_rectEnd; i++)
			{
				SDL_FRect rect = m_viewport.to_screen<dg::Rect, SDL_FRect>(m_output.m_rooms[i]);
				SDL_RenderRect(m_renderer, &rect);
			}

			SDL_SetRenderDrawColor(m_renderer, 0x80, 0, 0x80, 0xFF);
			for (int i = 0; i < 4; i++)
			{
				if (room.m_links[i] == &dg::impl::Vertex::s_sentinel)
					continue;

				SDL_FPoint p1 = m_viewport.to_screen(SDL_FPoint{ room.m_entrances[i].x + 0.5f, room.m_entrances[i].y + 0.5f });
				SDL_FPoint p2 = m_viewport.to_screen(SDL_FPoint{ room.m_links[i]->m_pos.x + 0.5f, room.m_links[i]->m_pos.y + 0.5f });

				SDL_RenderLine(m_renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}

		SDL_SetRenderDrawColor(m_renderer, 0x50, 0x50, 0x50, 0xFF);
		for (dg::impl::Vertex& vertex : m_generator.m_vertices)
		{
			SDL_FPoint p1 = m_viewport.to_screen(SDL_FPoint{ static_cast<float>(vertex.m_pos.x + 0.5f), static_cast<float>(vertex.m_pos.y + 0.5f) });
			const int end = (m_input.m_seed & 0b10) + 2;

			for (int i = m_input.m_seed & 0b10; i < end; i++)
			{
				dg::impl::Vertex* const vertex2 = vertex.m_links[i];
				if (vertex2 == &dg::impl::Vertex::s_sentinel || vertex2->ToRoom() != nullptr)
					continue;

				SDL_FPoint p2 = m_viewport.to_screen(SDL_FPoint{ static_cast<float>(vertex2->m_pos.x + 0.5f), static_cast<float>(vertex2->m_pos.y + 0.5f) });
				SDL_RenderLine(m_renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}

		for (dg::impl::Room& room : m_generator.m_rooms)
		{
			SDL_SetRenderDrawColor(m_renderer, 0, 0xC0, 0, 0xFF);
			SDL_FRect rect = m_viewport.to_screen(SDL_FRect{ static_cast<float>(room.m_pos.x), static_cast<float>(room.m_pos.y), 1, 1 });

			SDL_RenderFillRect(m_renderer, &rect);
			SDL_SetRenderDrawColor(m_renderer, 0x80, 0, 0x80, 0xFF);

			for (dg::Point& point : room.m_entrances)
			{
				rect.x = static_cast<float>(point.x);
				rect.y = static_cast<float>(point.y);
				rect.w = 1.0f; rect.h = 1.0f;

				rect = m_viewport.to_screen(rect);
				SDL_RenderFillRect(m_renderer, &rect);
			}
		}

		SDL_SetRenderDrawColor(m_renderer, 0, 0xC0, 0, 0xFF);
		for (dg::impl::Vertex& vertex : m_generator.m_vertices)
		{
			bool notEmpty = false;
			for (const auto& link : vertex.m_links)
				notEmpty |= link != &dg::impl::Vertex::s_sentinel;

			if (notEmpty)
			{
				SDL_FRect rect = m_viewport.to_screen(SDL_FRect{ static_cast<float>(vertex.m_pos.x), static_cast<float>(vertex.m_pos.y), 1, 1 });
				SDL_RenderFillRect(m_renderer, &rect);
			}
		}
	}
	else
	{
		if (m_visRooms)
		{
			SDL_SetRenderDrawColor(m_renderer, 0, 0xAA, 0xAA, 0xFF);
			for (dg::Rect& room : m_output.m_rooms)
			{
				SDL_FRect rect = m_viewport.to_screen<dg::Rect, SDL_FRect>(room);
				SDL_RenderRect(m_renderer, &rect);
			}
		}

		if (m_visPaths)
		{
			SDL_SetRenderDrawColor(m_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			for (auto& [origin, offset] : m_output.m_paths)
			{
				SDL_FPoint p1 = { origin.x + 0.5f, origin.y + 0.5f };
				SDL_FPoint p2 = { offset.x + p1.x, offset.y + p1.y };

				p1 = m_viewport.to_screen(p1);
				p2 = m_viewport.to_screen(p2);

				SDL_RenderLine(m_renderer, p1.x, p1.y, p2.x, p2.y);
			}
		}

		if (m_visEntrances)
		{
			SDL_SetRenderDrawColor(m_renderer, 0x80, 0, 0x80, 0xFF);
			for (dg::Point& entrance : m_output.m_entrances)
			{
				SDL_FRect rect = m_viewport.to_screen(SDL_FRect{ static_cast<float>(entrance.x), static_cast<float>(entrance.y), 1, 1 });
				SDL_RenderFillRect(m_renderer, &rect);
			}
		}
	}
}

void Application::draw()
{
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();

	ImGui::NewFrame();
	ImGui::Begin("Generator");

	if (ImGui::Button("Reset"))
	{
		load_defaults();
		m_viewport.reset();
		m_generateReq = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Next"))
	{
		m_input.m_seed++;
		m_generateReq = true;
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	m_generateReq |= ImGui::InputScalar("Seed", ImGuiDataType_U64, &m_input.m_seed);
	m_generateReq |= ImGui::SliderFloat("Size factor", &m_factor, 0.1f, 10.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
	m_generateReq |= ImGui::SliderInt("Minimum depth", &m_input.m_minDepth, 1, 12);
	m_generateReq |= ImGui::SliderInt("Maximum depth", &m_input.m_maxDepth, 1, 12);
	m_generateReq |= ImGui::SliderInt("Space interdistance", &m_input.m_spaceInterdistance, 0, 10);
	m_generateReq |= ImGui::SliderFloat("Space randomness", &m_input.m_spaceSizeRandomness, 0.0f, 1.0f);
	m_generateReq |= ImGui::SliderInt("Sparse area depth", &m_input.m_sparseAreaDepth, 1, 12);
	m_generateReq |= ImGui::SliderFloat("Sparse area density", &m_input.m_sparseAreaDens, 0.0f, 1.0f);
	m_generateReq |= ImGui::SliderFloat("Sparse area probability", &m_input.m_sparseAreaProb, 0.0f, 1.0f);
	m_generateReq |= ImGui::SliderFloat("Minimum room size", &m_input.m_minRoomSize, 0.0f, 1.0f);
	m_generateReq |= ImGui::SliderFloat("Maximum room size", &m_input.m_maxRoomSize, 0.0f, 1.0f);
	m_generateReq |= ImGui::SliderFloat("Double room probability", &m_input.m_doubleRoomProb, 0.0f, 1.0f);
	m_generateReq |= ImGui::SliderFloat("Heuristic", &m_input.m_heuristicFactor, 0.0f, 1.0f);
	m_generateReq |= ImGui::SliderFloat("Path cost factor", &m_input.m_pathCostFactor, 0.0f, 1.0f);
	m_generateReq |= ImGui::SliderInt("Extra path count", &m_input.m_extraPathCount, 0, 50);
	m_generateReq |= ImGui::SliderInt("Extra path depth", &m_input.m_extraPathDepth, 1, 12);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (m_error.empty())
	{
		ImGui::Text("Status: OK");
		ImGui::Text("Rooms: %d", static_cast<int>(m_output.m_rooms.size()));
		ImGui::Text("Paths: %d", static_cast<int>(m_output.m_paths.size()));
		ImGui::Text("Entrances: %d", static_cast<int>(m_output.m_entrances.size()));
	}
	else
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Status: Invalid configuration");

	ImGui::End();
	ImGui::Begin("Visualization");

	m_renderReq |= ImGui::Checkbox("Debug view", &m_debugView);
	if (!m_debugView)
	{
		m_renderReq |= ImGui::Checkbox("Room visibility", &m_visRooms);
		m_renderReq |= ImGui::Checkbox("Path visibility", &m_visPaths);
		m_renderReq |= ImGui::Checkbox("Entrance visibility", &m_visEntrances);
	}

	ImGui::End();
	ImGui::Render();

	SDL_SetRenderTarget(m_renderer, nullptr);
	SDL_RenderTexture(m_renderer, m_dungeon, nullptr, nullptr);
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
	SDL_RenderPresent(m_renderer);

	if (auto crr = clk_t::now(); crr < m_nextRefresh)
		std::this_thread::sleep_for(m_nextRefresh - crr);
	else if (crr > m_nextRefresh + m_frameDelay * 5)
		m_nextRefresh = crr;

	m_nextRefresh += m_frameDelay;
}

Application::Application()
{
	if (s_instances++ == 0)
		SDL_Init(SDL_INIT_VIDEO);
}

Application::~Application()
{
	cleanup();
	if (--s_instances == 0)
		SDL_Quit();
}

void Application::run()
{
	init();
	while (m_running)
	{
		handle_events();
		draw();
	}

	cleanup();
}
