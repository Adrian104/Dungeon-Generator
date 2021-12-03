#pragma once

class Animator
{
	public:
	typedef std::chrono::steady_clock clock_type;
	typedef clock_type::duration duration_type;
	typedef clock_type::time_point time_type;

	enum class LoopMode { ONLY_FORWARD, NO_LOOP, LOOP, BOUNCE_BACK };
	enum class DirMode { FORWARD, BACKWARD, TOGGLE, SWITCH, TOGGLE_OR_SWITCH, AUTO, NO_CHANGE };

	private:
	static constexpr auto zero = duration_type::zero();

	bool m_playing = false;
	bool m_backward = false;
	duration_type m_elapsed;
	time_type m_prevTimePoint;

	void ToggleState();

	public:
	LoopMode m_loopMode;
	duration_type m_totalTime;

	Animator(duration_type p_totalTime, LoopMode p_loopMode = LoopMode::NO_LOOP);

	void Stop();
	void Pause();
	void Update();

	void Play(DirMode p_dirMode);

	float GetPhase() const;
	bool IsPlaying() const;
	bool IsBackward() const;
	bool IsElapsedMin() const;
	bool IsElapsedMax() const;
};

inline void Animator::Stop()
{
	m_playing = false;
	m_backward = false;
	m_elapsed = zero;
}

inline void Animator::Pause()
{
	m_playing = false;
}

inline float Animator::GetPhase() const
{
	return m_elapsed.count() / float(m_totalTime.count());
}

inline bool Animator::IsPlaying() const
{
	return m_playing;
}

inline bool Animator::IsBackward() const
{
	return m_backward;
}

inline bool Animator::IsElapsedMin() const
{
	return m_elapsed <= zero;
}

inline bool Animator::IsElapsedMax() const
{
	return m_elapsed >= m_totalTime;
}