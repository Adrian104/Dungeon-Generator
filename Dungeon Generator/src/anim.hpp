#pragma once

class Animator
{
	float phase;
	int8_t state;

	std::chrono::duration<float> totalTime;
	std::chrono::steady_clock::time_point prevTimePoint;

	public:
	Animator(std::chrono::duration<float> pTotalTime);

	void UpdateAnim();
	void ToggleAnim(int8_t dir = 0);

	inline float GetAnimPhase() const { return phase; }
	inline int8_t GetAnimState() const { return state; }
	inline bool IsAnimating() const { return state != 0; }
};