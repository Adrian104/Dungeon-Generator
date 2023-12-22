// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "help.hpp"
#include <vector>

Help::Help() : Animator(g_helpAnimTime)
{
	m_renderOutput = m_app.CreateTexture(m_app.GetWidth(), m_app.GetHeight());
	ScheduleRendering();
}

Help::~Help() { SDL_DestroyTexture(m_renderOutput); }

void Help::Draw()
{
	if (IsElapsedMax())
		return;

	SDL_SetTextureAlphaMod(m_renderOutput, static_cast<Uint8>(0xFF - static_cast<int>(0xFF * GetPhase())));
	SDL_RenderCopy(m_app.GetRenderer(), m_renderOutput, nullptr, nullptr);
}

void Help::Render()
{
	SDL_Renderer* const renderer = m_app.GetRenderer();

	SDL_SetRenderTarget(renderer, m_renderOutput);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	Texture keyText(m_app.RenderText("KEY", 0, TTF_STYLE_BOLD));
	Texture descText(m_app.RenderText("DESCRIPTION", 0, TTF_STYLE_BOLD));
	Texture pressText(m_app.RenderText("PRESS  ANY  KEY  TO  CONTINUE...", 0, TTF_STYLE_BOLD));

	int sumHeight = std::max(keyText.GetHeight(), descText.GetHeight());
	int maxWidthA = keyText.GetWidth();
	int maxWidthB = descText.GetWidth();

	std::vector<std::pair<Texture, Texture>> data;
	for (const auto& [button, desc] : g_buttonDescriptions)
	{
		auto& [a, b] = data.emplace_back();

		a.Set(m_app.RenderText(button, 0, TTF_STYLE_NORMAL));
		b.Set(m_app.RenderText(desc, 0, TTF_STYLE_NORMAL));

		sumHeight += std::max(a.GetHeight(), b.GetHeight());

		if (maxWidthA < a.GetWidth())
			maxWidthA = a.GetWidth();

		if (maxWidthB < b.GetWidth())
			maxWidthB = b.GetWidth();
	}

	const int xStart = (m_app.GetWidth() - maxWidthA - maxWidthB - g_helpXOffset) >> 1;
	const int keyAxis = xStart + maxWidthA;
	const int descAxis = xStart + maxWidthA + g_helpXOffset;

	int yPos = (m_app.GetHeight() - sumHeight - g_helpYOffset - g_helpAnyKeyOffset - pressText.GetHeight()) >> 1;
	SDL_Rect dest;

	dest = { keyAxis - keyText.GetWidth(), yPos, keyText.GetWidth(), keyText.GetHeight() };
	SDL_RenderCopy(renderer, keyText.Get(), nullptr, &dest);

	dest = { descAxis, yPos, descText.GetWidth(), descText.GetHeight() };
	SDL_RenderCopy(renderer, descText.Get(), nullptr, &dest);

	yPos += std::max(keyText.GetHeight(), descText.GetHeight()) + g_helpYOffset;
	for (auto& [a, b] : data)
	{
		dest = { keyAxis - a.GetWidth(), yPos, a.GetWidth(), a.GetHeight() };
		SDL_RenderCopy(renderer, a.Get(), nullptr, &dest);

		dest = { descAxis, yPos, b.GetWidth(), b.GetHeight() };
		SDL_RenderCopy(renderer, b.Get(), nullptr, &dest);

		yPos += std::max(a.GetHeight(), b.GetHeight());
	}

	dest = { (m_app.GetWidth() - pressText.GetWidth()) >> 1, yPos + g_helpAnyKeyOffset, pressText.GetWidth(), pressText.GetHeight() };
	SDL_RenderCopy(renderer, pressText.Get(), nullptr, &dest);
}

void Help::HandleEvent(SDL_Event& sdlEvent)
{
	if (Widget::s_active == this && sdlEvent.type == SDL_KEYDOWN)
	{
		Widget::s_active = nullptr;
		Play(DirMode::FORWARD);
	}
}