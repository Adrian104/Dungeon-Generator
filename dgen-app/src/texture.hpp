// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <utility>

class Texture
{
	int m_width = 0;
	int m_height = 0;

	SDL_Texture* m_texture = nullptr;

	void Query();

public:
	Texture() = default;
	Texture(SDL_Texture*&& texture);
	~Texture();

	Texture(const Texture& ref) = delete;
	Texture& operator=(const Texture& ref) = delete;

	Texture(Texture&& ref) noexcept;
	Texture& operator=(Texture&& ref) noexcept;

	void Clear();
	void Set(SDL_Texture*&& texture);
	SDL_Texture* Release();

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	SDL_Texture* Get() const { return m_texture; }
};

inline void Texture::Query()
{
	if (m_texture != nullptr)
		SDL_QueryTexture(m_texture, nullptr, nullptr, &m_width, &m_height);
}

inline Texture::Texture(SDL_Texture*&& texture)
	: m_texture(std::exchange(texture, nullptr)) { Query(); }

inline Texture::~Texture() { if (m_texture != nullptr) SDL_DestroyTexture(m_texture); }

inline Texture::Texture(Texture&& ref) noexcept
	: m_width(std::exchange(ref.m_width, 0)), m_height(std::exchange(ref.m_height, 0)), m_texture(std::exchange(ref.m_texture, nullptr)) {}

inline Texture& Texture::operator=(Texture&& ref) noexcept
{
	if (m_texture != nullptr)
		SDL_DestroyTexture(m_texture);

	m_width = std::exchange(ref.m_width, 0);
	m_height = std::exchange(ref.m_height, 0);
	m_texture = std::exchange(ref.m_texture, nullptr);

	return *this;
}

inline void Texture::Clear()
{
	m_width = 0;
	m_height = 0;

	if (m_texture != nullptr)
	{
		SDL_DestroyTexture(m_texture);
		m_texture = nullptr;
	}
}

inline void Texture::Set(SDL_Texture*&& texture)
{
	if (m_texture != nullptr)
		SDL_DestroyTexture(m_texture);

	m_width = 0;
	m_height = 0;
	m_texture = std::exchange(texture, nullptr);

	Query();
}

inline SDL_Texture* Texture::Release()
{
	m_width = 0;
	m_height = 0;

	return std::exchange(m_texture, nullptr);
}