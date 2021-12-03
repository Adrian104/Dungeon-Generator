#include "pch.hpp"
#include "vport.hpp"

void Viewport::Move(int p_xMouse, int p_yMouse)
{
	m_xOffset += (m_xStart - p_xMouse) / m_scale;
	m_yOffset += (m_yStart - p_yMouse) / m_scale;

	m_xStart = p_xMouse;
	m_yStart = p_yMouse;
}

void Viewport::Scale(int p_xMouse, int p_yMouse, float p_factor)
{
	const float before = 1 / m_scale;
	m_scale *= (m_scaleStep * p_factor) + 1;

	const float after = 1 / m_scale;
	const float diff = before - after;

	m_xOffset += p_xMouse * diff;
	m_yOffset += p_yMouse * diff;
}

bool Viewport::Update(SDL_Event& p_sdlEvent)
{
	int xMouse, yMouse;
	SDL_GetMouseState(&xMouse, &yMouse);

	switch (p_sdlEvent.type)
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
		Scale(xMouse, yMouse, static_cast<float>(p_sdlEvent.wheel.y));
		break;

	default:
		return false;
	}

	return true;
}