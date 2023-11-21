// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include "../app.hpp"
#include "mod.hpp"
#include <string>

class Input : public Widget, public Animator
{
	bool m_invalid = false;
	std::string m_input;
	Modifier* m_mod = nullptr;
	SDL_Texture* m_renderOutput;

public:
	Input();
	~Input();

	void Set(Modifier* mod);
	void Draw() override;
	void Render() override;
	void HandleEvent(SDL_Event& sdlEvent) override;
};