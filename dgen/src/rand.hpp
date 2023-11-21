// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include <stdint.h>
#include <utility>

namespace dg::impl
{
	class Random
	{
		using uint32p_t = std::pair<uint32_t, uint32_t>;
		uint64_t m_state[4];

	public:
		Random() { Seed(); }
		Random(const uint64_t seed) { Seed(seed); }

		void Seed(uint64_t seed = 0);

		bool GetBit();
		float GetFP32();
		double GetFP64();
		uint32_t Get32();
		uint64_t Get64();
		uint32p_t Get32P();
	};

	inline void Random::Seed(uint64_t seed)
	{
		// Algorithm: SplitMix64
		// Source: https://prng.di.unimi.it/splitmix64.c

		for (uint64_t& state : m_state)
		{
			uint64_t z = (seed += 0x9e3779b97f4a7c15);
			z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
			z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
			state = z ^ (z >> 31);
		}
	}

	inline bool Random::GetBit()
	{
		const auto [a, b] = Get32P();
		return a < b;
	}

	inline float Random::GetFP32()
	{
		union { uint32_t i; float f; } u{};
		u.i = (Get32() >> 9) | 0x3F800000;
		return u.f - 1.0f;
	}

	inline double Random::GetFP64()
	{
		union { uint64_t i; double d; } u{};
		u.i = (Get64() >> 12) | 0x3FF0000000000000;
		return u.d - 1.0;
	}

	inline uint32_t Random::Get32()
	{
		return static_cast<uint32_t>(Get64());
	}

	inline uint64_t Random::Get64()
	{
		// Algorithm: xoshiro256+
		// Source: https://prng.di.unimi.it/xoshiro256plus.c

		const uint64_t result = m_state[0] + m_state[3];
		const uint64_t t = m_state[1] << 17;

		m_state[2] ^= m_state[0];
		m_state[3] ^= m_state[1];
		m_state[1] ^= m_state[2];
		m_state[0] ^= m_state[3];

		m_state[2] ^= t;
		m_state[3] = (m_state[3] << 45) | (m_state[3] >> (64 - 45));

		return result;
	}

	inline Random::uint32p_t Random::Get32P()
	{
		const uint64_t r = Get64();
		return uint32p_t(static_cast<uint32_t>(r >> 32), static_cast<uint32_t>(r));
	}
}