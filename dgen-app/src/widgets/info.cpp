// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "info.hpp"

void Info::Clear()
{
	if (m_renderOutput != nullptr)
	{
		SDL_DestroyTexture(m_renderOutput);
		m_renderOutput = nullptr;
	}
}

void Info::Draw()
{
	if (m_renderOutput == nullptr)
		return;

	const SDL_Rect dest = { m_app.GetWidth() - m_width, 0, m_width, m_height };

	SDL_SetTextureAlphaMod(m_renderOutput, static_cast<Uint8>(0xFF - static_cast<int>(0xFF * GetPhase())));
	SDL_RenderCopy(m_app.GetRenderer(), m_renderOutput, nullptr, &dest);
}

void Info::Set(const std::string& msg)
{
	Clear();

	SDL_Renderer* const renderer = m_app.GetRenderer();
	Texture text(m_app.RenderText(msg, 0));

	m_width = text.GetWidth();
	m_height = text.GetHeight();
	m_renderOutput = m_app.CreateTexture(m_width, m_height);

	SDL_SetRenderTarget(renderer, m_renderOutput);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, text.Get(), nullptr, nullptr);
}

void Info::HandleEvent(SDL_Event& sdlEvent)
{
	if (sdlEvent.type == SDL_KEYDOWN)
	{
		const SDL_Keycode keycode = sdlEvent.key.keysym.sym;
		if (keycode == SDLK_i)
			Play(DirMode::AUTO);
	}
}