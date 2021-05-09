#include "anim.hpp"

Animator::Animator(std::chrono::duration<float> pTotalTime) : phase(0), state(0), totalTime(pTotalTime) {}

void Animator::UpdateAnim()
{
	std::chrono::steady_clock::time_point crrTimePoint = std::chrono::steady_clock::now();
	const float diff = (crrTimePoint - prevTimePoint) / totalTime;

	phase += diff * state;
	const bool overOne = phase > 1;

	if (phase < 0 || overOne)
	{
		phase = float(overOne);
		state = 0;
	}

	prevTimePoint = crrTimePoint;
}

void Animator::ToggleAnim(int8_t dir)
{
	if (dir == 0)
	{
		if (state == 0) state = phase < 0.5f ? 1 : -1;
		else state = -state;
	}
	else state = dir;

	prevTimePoint = std::chrono::steady_clock::now();
}