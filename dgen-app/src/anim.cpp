// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "anim.hpp"

void Animator::ToggleState()
{
	switch (m_loopMode)
	{
	case LoopMode::ONLY_FORWARD:
		m_elapsed = s_zero;
		[[fallthrough]];

	case LoopMode::NO_LOOP:
		m_playing = false;
		break;

	case LoopMode::LOOP:
		m_backward = !m_backward;
		break;

	case LoopMode::BOUNCE_BACK:
		if (m_backward) m_playing = false;
		else m_backward = true;
	}
}

Animator::Animator(duration_type totalTime, LoopMode loopMode)
	: m_elapsed(), m_prevTimePoint(), m_totalTime(totalTime), m_loopMode(loopMode) {}

void Animator::Update()
{
	if (!m_playing) return;

	const time_type crrTimePoint = clock_type::now();
	const duration_type deltaTime = crrTimePoint - m_prevTimePoint;

	m_prevTimePoint = crrTimePoint;

	if (m_backward)
	{
		m_elapsed -= deltaTime;
		if (m_elapsed <= s_zero)
		{
			m_elapsed = s_zero;
			ToggleState();
		}
	}
	else
	{
		m_elapsed += deltaTime;
		if (m_elapsed >= m_totalTime)
		{
			m_elapsed = m_totalTime;
			ToggleState();
		}
	}
}

void Animator::Play(DirMode dirMode)
{
	if (m_loopMode == LoopMode::ONLY_FORWARD)
		m_backward = false;
	else switch (dirMode)
	{
	case DirMode::FORWARD:
		m_backward = false;
		break;

	case DirMode::BACKWARD:
		m_backward = true;
		break;

	case DirMode::TOGGLE:
		m_backward = !m_backward;
		break;

	case DirMode::SWITCH:
		if (m_elapsed <= s_zero) m_backward = false;
		else if (m_elapsed >= m_totalTime) m_backward = true;
		break;

	case DirMode::TOGGLE_OR_SWITCH:
		if (m_elapsed <= s_zero) m_backward = false;
		else if (m_elapsed >= m_totalTime) m_backward = true;
		else m_backward = !m_backward;
		break;

	case DirMode::AUTO:
		if (m_elapsed <= s_zero) m_backward = false;
		else if (m_elapsed >= m_totalTime) m_backward = true;
		else if (m_playing) m_backward = !m_backward;
		[[fallthrough]];

	default:
		break;
	}

	m_playing = true;
	m_prevTimePoint = clock_type::now();
}