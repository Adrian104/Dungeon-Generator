// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include "../app.hpp"
#include "mod.hpp"
#include <vector>

class Menu : public Widget, public Animator
{
	int m_selection = 0;
	SDL_Texture* m_renderOutput;
	std::vector<Modifier*> m_mods;

public:
	Menu();
	~Menu();

	void Draw() override;
	void Render() override;
	void HandleEvent(SDL_Event& sdlEvent) override;

	template <typename Type, typename... Args>
	void Add(Args&&... args) { m_mods.push_back(new Type(std::forward<Args>(args)...)); }
};