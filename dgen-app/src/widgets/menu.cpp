// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "menu.hpp"
#include "input.hpp"

Menu::Menu() : Animator(g_menuAnimTime)
{
	m_renderOutput = m_app.CreateTexture(g_menuWidth, m_app.GetHeight());
}

Menu::~Menu()
{
	for (auto& mod : m_mods) delete mod;
	SDL_DestroyTexture(m_renderOutput);
}

void Menu::Draw()
{
	const SDL_Rect dest = { static_cast<int>(-g_menuWidth * GetPhase()), 0, g_menuWidth, m_app.GetHeight() };
	SDL_RenderCopy(m_app.GetRenderer(), m_renderOutput, nullptr, &dest);
}

void Menu::Render()
{
	SDL_Renderer* const renderer = m_app.GetRenderer();

	SDL_SetRenderTarget(renderer, m_renderOutput);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xCC);
	SDL_RenderClear(renderer);

	SDL_Rect rect = { g_menuWidth - g_menuOutlineWidth, 0, g_menuOutlineWidth, m_app.GetHeight() };

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(renderer, &rect);

	Texture text(m_app.RenderText(g_title, 0, TTF_STYLE_BOLD));

	rect = { (g_menuWidth - text.GetWidth()) >> 1, g_menuTitleMargin, text.GetWidth(), text.GetHeight() };
	SDL_RenderCopy(renderer, text.Get(), nullptr, &rect);

	rect.x -= g_menuMargin;
	rect.y -= g_menuMargin;
	rect.w += g_menuMargin << 1;
	rect.h += g_menuMargin << 1;

	SDL_RenderDrawRect(renderer, &rect);

	int pos = g_menuTitleMargin << 1;
	Modifier* const selected = m_mods.at(m_selection);

	for (Modifier* mod : m_mods)
	{
		const uint8_t color = mod == selected ? 0 : 0xFF;

		text.Set(m_app.RenderText(mod->m_name, 0, TTF_STYLE_BOLD, { color, 0xFF, color, 0xFF }));
		rect = { (g_menuWidth - text.GetWidth()) >> 1, pos, text.GetWidth(), text.GetHeight() };
		pos += g_menuInternalOffset;

		SDL_RenderCopy(renderer, text.Get(), nullptr, &rect);

		text.Set(m_app.RenderText(mod->GetValue(), 0));
		rect = { (g_menuWidth - text.GetWidth()) >> 1, pos, text.GetWidth(), text.GetHeight() };
		pos += g_menuExternalOffset;

		SDL_RenderCopy(renderer, text.Get(), nullptr, &rect);
	}
}

void Menu::HandleEvent(SDL_Event& sdlEvent)
{
	if (sdlEvent.type == SDL_KEYDOWN)
	{
		const SDL_Keycode keycode = sdlEvent.key.keysym.sym;
		if (keycode == SDLK_m)
		{
			Play(DirMode::AUTO);
			return;
		}

		if (IsElapsedMax())
			return;

		switch (keycode)
		{
		case SDLK_UP:
			if (--m_selection < 0)
				m_selection = static_cast<int>(m_mods.size()) - 1;
			break;

		case SDLK_DOWN:
			if (++m_selection >= static_cast<int>(m_mods.size()))
				m_selection = 0;
			break;

		case SDLK_RETURN:
			m_app.AccessWidget<Input>().Set(m_mods.at(m_selection));
			break;

		case SDLK_RIGHT:
		case SDLK_EQUALS:
			m_mods.at(m_selection)->Increment();
			m_app.ScheduleGeneration(Application::SeedMode::KEEP);
			break;

		case SDLK_LEFT:
		case SDLK_MINUS:
			m_mods.at(m_selection)->Decrement();
			m_app.ScheduleGeneration(Application::SeedMode::KEEP);
			break;

		default:
			return;
		}

		ScheduleRendering();
	}
}