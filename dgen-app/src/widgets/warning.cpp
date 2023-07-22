// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "warning.hpp"

void Warning::Clear()
{
	if (m_renderOutput != nullptr)
	{
		SDL_DestroyTexture(m_renderOutput);
		m_renderOutput = nullptr;
	}
}

void Warning::Draw()
{
	if (m_renderOutput == nullptr)
		return;

	const SDL_Rect dest = { m_app.GetWidth() - m_width, m_app.GetHeight() - static_cast<int>(m_height * GetPhase()), m_width, m_height };
	SDL_RenderCopy(m_app.GetRenderer(), m_renderOutput, nullptr, &dest);
}

void Warning::Set(const std::string& msg)
{
	const size_t hash = std::hash<std::string>()(msg);
	if (m_hash == hash) return;

	m_hash = hash;
	if (msg.empty())
	{
		Play(DirMode::BACKWARD);
		return;
	}

	Play(DirMode::FORWARD);
	Clear();

	SDL_Renderer* const renderer = m_app.GetRenderer();
	Texture text(m_app.RenderText(msg, 0, TTF_STYLE_BOLD));

	m_width = text.GetWidth() + (g_warningMargin << 1);
	m_height = text.GetHeight() + (g_warningMargin << 1);
	m_renderOutput = m_app.CreateTexture(m_width, m_height);

	SDL_SetRenderTarget(renderer, m_renderOutput);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
	SDL_RenderDrawRect(renderer, nullptr);

	const SDL_Rect dest = { g_warningMargin, g_warningMargin, text.GetWidth(), text.GetHeight() };
	SDL_RenderCopy(renderer, text.Get(), nullptr, &dest);
}