// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include "../app.hpp"

class Help : public Widget, public Animator
{
	SDL_Texture* m_renderOutput;

public:
	Help();
	~Help();

	void Draw() override;
	void Render() override;
	void HandleEvent(SDL_Event& sdlEvent) override;
};