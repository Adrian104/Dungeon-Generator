// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "input.hpp"
#include "menu.hpp"

Input::Input() : Animator(g_inputAnimTime) { m_renderOutput = m_app.CreateTexture(g_inputWidth, g_inputHeight); }
Input::~Input() { SDL_DestroyTexture(m_renderOutput); }

void Input::Set(Modifier* mod)
{
	m_input.clear();
	m_mod = mod;
	m_invalid = false;
	s_active = this;

	ScheduleRendering();
	Play(DirMode::FORWARD);
}

void Input::Draw()
{
	if (IsElapsedMin())
		return;

	const int width = static_cast<int>(g_inputWidth * GetPhase());
	const SDL_Rect dest =
	{
		(m_app.GetWidth() - width) >> 1, (m_app.GetHeight() - g_inputHeight) >> 1,
		width, g_inputHeight
	};

	SDL_RenderCopy(m_app.GetRenderer(), m_renderOutput, nullptr, &dest);
	SDL_SetRenderDrawColor(m_app.GetRenderer(), 0xFF, 0, 0, 0xFF);
	SDL_RenderDrawRect(m_app.GetRenderer(), &dest);
}

void Input::Render()
{
	if (m_mod == nullptr)
		return;

	SDL_Renderer* const renderer = m_app.GetRenderer();

	SDL_SetRenderTarget(renderer, m_renderOutput);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	Texture text(m_app.RenderText("Variable name:", 0, TTF_STYLE_BOLD));

	const int step = g_inputHeight >> 2;
	int offset = step - (text.GetHeight() >> 1);
	static constexpr SDL_Color s_colors[2] = { { 0, 0xFF, 0, 0xFF }, { 0xFF, 0, 0, 0xFF } };

	SDL_Rect dest = { g_inputXOffset1, offset, text.GetWidth(), text.GetHeight() };
	SDL_RenderCopy(renderer, text.Get(), nullptr, &dest);

	text.Set(m_app.RenderText(m_mod->m_name, 0, TTF_STYLE_NORMAL));
	dest = { g_inputXOffset2, offset, text.GetWidth(), text.GetHeight() };
	SDL_RenderCopy(renderer, text.Get(), nullptr, &dest);

	text.Set(m_app.RenderText("Current value:", 0, TTF_STYLE_BOLD));
	dest = { g_inputXOffset1, offset += step, text.GetWidth(), text.GetHeight() };
	SDL_RenderCopy(renderer, text.Get(), nullptr, &dest);

	text.Set(m_app.RenderText(m_mod->GetValue(), 0, TTF_STYLE_NORMAL));
	dest = { g_inputXOffset2, offset, text.GetWidth(), text.GetHeight() };
	SDL_RenderCopy(renderer, text.Get(), nullptr, &dest);

	text.Set(m_app.RenderText("New value:", 0, TTF_STYLE_BOLD));
	dest = { g_inputXOffset1, offset += step, text.GetWidth(), text.GetHeight() };
	SDL_RenderCopy(renderer, text.Get(), nullptr, &dest);

	text.Set(m_app.RenderText(m_input + "_", 0, TTF_STYLE_NORMAL, s_colors[m_invalid]));
	dest = { g_inputXOffset2, offset, text.GetWidth(), text.GetHeight() };
	SDL_RenderCopy(renderer, text.Get(), nullptr, &dest);
}

void Input::HandleEvent(SDL_Event& sdlEvent)
{
	if (s_active == this && sdlEvent.type == SDL_KEYDOWN)
	{
		const SDL_Keycode key = SDL_GetKeyFromScancode(sdlEvent.key.keysym.scancode);
		if (key & ~0xFF) return;

		m_invalid = false;
		switch (key)
		{
		case SDLK_RETURN:
			if (m_input.empty()) break;
			try { m_mod->SetValue(m_input); m_mod->Check(); }
			catch (...) { m_invalid = true; break; }

			m_app.AccessWidget<Menu>().ScheduleRendering();
			m_app.ScheduleGeneration(Application::SeedMode::KEEP);
			[[fallthrough]];

		case SDLK_ESCAPE:
			m_mod = nullptr;
			s_active = nullptr;
			Play(DirMode::BACKWARD);
			break;

		case SDLK_BACKSPACE:
			if (!m_input.empty())
				m_input.pop_back();
			break;

		default:
			m_input += static_cast<char>(key);
		}

		ScheduleRendering();
	}
}