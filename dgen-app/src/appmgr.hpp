// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include "texture.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <map>
#include <string>

class AppManager
{
	int m_width = 0;
	int m_height = 0;

	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	std::map<int, TTF_Font*> m_fonts;

public:
	AppManager();
	~AppManager();

	AppManager(const AppManager& ref) = delete;
	AppManager& operator=(const AppManager& ref) = delete;

	AppManager(AppManager&& ref) noexcept = delete;
	AppManager& operator=(AppManager&& ref) noexcept = delete;

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	SDL_Window* GetWindow() const { return m_window; }
	SDL_Renderer* GetRenderer() const { return m_renderer; }

	void ResetAppManager();

	void UnloadAllFonts();
	void UnloadFont(int id);
	void LoadFont(int id, int size, const std::string& path);
	void LoadFont(int id, int size, const void* data, int dataSize);

	void DestroyWindow();
	void CreateWindow(const std::string& title, int width, int height, Uint32 flags = 0);

	SDL_Texture* CreateTexture(int width, int height, bool blend = true) const;
	SDL_Texture* RenderText(const std::string& str, int id, int style = TTF_STYLE_NORMAL, const SDL_Color& color = { 0xFF, 0xFF, 0xFF, 0xFF }) const;
};