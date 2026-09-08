// SPDX-FileCopyrightText: Copyright (c) 2026 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include "pch.hpp"
#include "vport.hpp"

using clk_t = std::chrono::steady_clock;

class Application
{
private:
	enum class View { DEBUG, GEOMETRY, TILEMAP };

	int m_width = 0;
	int m_height = 0;
	float m_factor = 1.0f;

	bool m_running = true;
	bool m_visRooms = true;
	bool m_visPaths = true;
	bool m_visEntrances = false;
	bool m_renderReq = true;
	bool m_generateReq = true;

	View m_view = View::GEOMETRY;
	Viewport m_viewport;
	std::string m_error;

	dg::Input m_input;
	dg::Output m_output;
	dg::Tilemap m_tilemap;
	dg::impl::Generator m_generator;

	std::vector<std::tuple<SDL_FRect, std::uint8_t, std::uint8_t, std::uint8_t>> m_tileCache;

	clk_t::duration m_frameDelay;
	clk_t::time_point m_nextRefresh;
	clk_t::duration m_dungeonGenTime;
	clk_t::duration m_cacheGenTime;

	ImGuiIO* m_io = nullptr;
	ImGuiContext* m_context = nullptr;
	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	SDL_Texture* m_dungeon = nullptr;

	inline static int s_instances = 0;

	void load_defaults();
	void init();
	void cleanup();
	void handle_events();
	void generate();
	void build_tile_cache();
	void render();
	void draw();

public:
	Application();
	~Application();

	void run();
};
