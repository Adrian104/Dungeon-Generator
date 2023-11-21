// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "appmgr.hpp"
#include <stdexcept>
#include <utility>

AppManager::AppManager()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
}

AppManager::~AppManager()
{
	ResetAppManager();

	TTF_Quit();
	SDL_Quit();
}

void AppManager::ResetAppManager()
{
	DestroyWindow();
	UnloadAllFonts();
}

void AppManager::UnloadAllFonts()
{
	for (auto& [id, ptr] : m_fonts)
		TTF_CloseFont(ptr);

	m_fonts.clear();
}

void AppManager::UnloadFont(int id)
{
	const auto iter = m_fonts.find(id);
	if (iter == m_fonts.end()) return;

	TTF_CloseFont(iter->second);
	m_fonts.erase(iter);
}

void AppManager::LoadFont(int id, int size, const std::string& path)
{
	UnloadFont(id);

	TTF_Font* const font = TTF_OpenFont(path.c_str(), size);
	if (font == nullptr) throw std::runtime_error(TTF_GetError());

	m_fonts.emplace(id, font);
}

void AppManager::LoadFont(int id, int size, const void* data, int dataSize)
{
	UnloadFont(id);

	SDL_RWops* const rwOps = SDL_RWFromConstMem(data, dataSize);
	if (rwOps == nullptr) throw std::runtime_error(SDL_GetError());

	TTF_Font* const font = TTF_OpenFontRW(rwOps, true, size);
	if (font == nullptr) throw std::runtime_error(TTF_GetError());

	m_fonts.emplace(id, font);
}

void AppManager::DestroyWindow()
{
	if (m_renderer != nullptr)
	{
		SDL_DestroyRenderer(m_renderer);
		m_renderer = nullptr;
	}

	if (m_window != nullptr)
	{
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}
}

void AppManager::CreateWindow(const std::string& title, int width, int height, Uint32 flags)
{
	DestroyWindow();

	m_width = width;
	m_height = height;

	m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (m_window == nullptr) throw std::runtime_error(SDL_GetError());

	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (m_renderer == nullptr) throw std::runtime_error(SDL_GetError());
}

SDL_Texture* AppManager::CreateTexture(int width, int height, bool blend) const
{
	SDL_Texture* const texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
	if (blend) SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	return texture;
}

SDL_Texture* AppManager::RenderText(const std::string& str, int id, int style, const SDL_Color& color) const
{
	TTF_Font* const font = m_fonts.at(id);
	TTF_SetFontStyle(font, style);

	SDL_Surface* const surface = TTF_RenderText_Blended(font, str.c_str(), color);

	if (surface == nullptr)
		throw std::runtime_error(TTF_GetError());

	SDL_Texture* const texture = SDL_CreateTextureFromSurface(m_renderer, surface);
	SDL_FreeSurface(surface);

	if (texture == nullptr)
		throw std::runtime_error(SDL_GetError());

	return texture;
}