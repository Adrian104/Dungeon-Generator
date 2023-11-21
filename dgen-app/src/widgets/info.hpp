// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include "../app.hpp"
#include <string>

class Info : public Widget, public Animator
{
	int m_width = 0;
	int m_height = 0;

	SDL_Texture* m_renderOutput = nullptr;

	void Clear();

public:
	Info() : Animator(g_infoAnimTime) {}
	~Info() { Clear(); }

	void Draw() override;
	void Set(const std::string& msg);
	void HandleEvent(SDL_Event& sdlEvent) override;
};