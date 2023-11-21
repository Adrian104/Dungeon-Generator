// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include <chrono>

class Animator
{
public:
	using clock_type = std::chrono::steady_clock;
	using duration_type = clock_type::duration;
	using time_type = clock_type::time_point;

	enum class LoopMode { ONLY_FORWARD, NO_LOOP, LOOP, BOUNCE_BACK };
	enum class DirMode { FORWARD, BACKWARD, TOGGLE, SWITCH, TOGGLE_OR_SWITCH, AUTO, KEEP };

private:
	static constexpr auto s_zero = duration_type::zero();

	bool m_playing = false;
	bool m_backward = false;
	duration_type m_elapsed;
	time_type m_prevTimePoint;

	void ToggleState();

public:
	duration_type m_totalTime;
	LoopMode m_loopMode;

	Animator(duration_type totalTime, LoopMode loopMode = LoopMode::NO_LOOP);

	void Stop();
	void Pause();
	void Update();

	float GetPhase() const;
	void Play(DirMode dirMode);

	bool IsPlaying() const { return m_playing; }
	bool IsBackward() const { return m_backward; }
	bool IsElapsedMin() const { return m_elapsed <= s_zero; }
	bool IsElapsedMax() const { return m_elapsed >= m_totalTime; }
};

inline void Animator::Stop()
{
	m_playing = false;
	m_backward = false;
	m_elapsed = s_zero;
}

inline void Animator::Pause()
{
	m_playing = false;
}

inline float Animator::GetPhase() const
{
	return m_elapsed.count() / static_cast<float>(m_totalTime.count());
}