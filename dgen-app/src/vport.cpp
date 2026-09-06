// SPDX-FileCopyrightText: Copyright (c) 2026 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "pch.hpp"
#include "vport.hpp"

void Viewport::move(float xmouse, float ymouse)
{
	m_xdisp += (m_xstart - xmouse) / m_scale;
	m_ydisp += (m_ystart - ymouse) / m_scale;

	m_xstart = xmouse;
	m_ystart = ymouse;
}

void Viewport::scale(float xmouse, float ymouse, float factor)
{
	const float before = 1.0f / m_scale;
	m_scale *= (m_scaleStep * factor) + 1.0f;

	const float diff = before - (1.0f / m_scale);

	m_xdisp += xmouse * diff;
	m_ydisp += ymouse * diff;
}

void Viewport::reset()
{
	m_scale = m_defScale;
	m_xdisp = 0.0f;
	m_ydisp = 0.0f;
}

bool Viewport::update(SDL_Event& event)
{
	float xmouse, ymouse;
	SDL_GetMouseState(&xmouse, &ymouse);

	switch (event.type)
	{
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		m_xstart = xmouse;
		m_ystart = ymouse;
		m_pressed = true;
		break;

	case SDL_EVENT_MOUSE_BUTTON_UP:
		m_pressed = false;
		break;

	case SDL_EVENT_MOUSE_MOTION:
		if (!m_pressed)
			return false;

		move(xmouse, ymouse);
		break;

	case SDL_EVENT_MOUSE_WHEEL:
		scale(xmouse, ymouse, event.wheel.y);
		break;

	default:
		return false;
	}

	return true;
}

void Viewport::set_scale_step(float step)
{
	m_scaleStep = step;
}

void Viewport::set_default_scale(float scale)
{
	m_defScale = scale;
}
