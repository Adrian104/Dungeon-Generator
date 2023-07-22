// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "vport.hpp"

void Viewport::Move(int xMouse, int yMouse)
{
	m_xOffset += (m_xStart - xMouse) / m_scale;
	m_yOffset += (m_yStart - yMouse) / m_scale;

	m_xStart = xMouse;
	m_yStart = yMouse;
}

void Viewport::Scale(int xMouse, int yMouse, float factor)
{
	const float before = 1 / m_scale;
	m_scale *= (m_scaleStep * factor) + 1;

	const float diff = before - (1 / m_scale);

	m_xOffset += xMouse * diff;
	m_yOffset += yMouse * diff;
}

bool Viewport::Update(SDL_Event& sdlEvent)
{
	int xMouse, yMouse;
	SDL_GetMouseState(&xMouse, &yMouse);

	switch (sdlEvent.type)
	{
	case SDL_MOUSEBUTTONDOWN:
		m_xStart = xMouse;
		m_yStart = yMouse;
		m_pressed = true;
		break;

	case SDL_MOUSEBUTTONUP:
		m_pressed = false;
		break;

	case SDL_MOUSEMOTION:
		if (!m_pressed) return false;
		Move(xMouse, yMouse);
		break;

	case SDL_MOUSEWHEEL:
		Scale(xMouse, yMouse, static_cast<float>(sdlEvent.wheel.y));
		break;

	default:
		return false;
	}

	return true;
}