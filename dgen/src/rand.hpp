// SPDX-FileCopyrightText: Copyright (c) 2026 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include <cassert>
#include <cstdint>
#include <ctime>
#include <utility>

namespace dg::impl
{
	template <typename T>
	inline T rotl(T x, int k)
	{
		return (x << k) | (x >> (64 - k));
	}

	class Random
	{
	private:
		std::uint64_t m_state[4];

	public:
		Random();
		Random(std::uint64_t seed);

		void init(std::uint64_t seed);

		bool flip();
		bool prob(float p);
		float fp32();

		std::uint64_t random();
		std::uint32_t rd32(std::uint64_t b);
		std::pair<std::uint32_t, std::uint32_t> rd32x2(std::uint64_t b1, std::uint64_t b2);
	};

	inline Random::Random()
	{
		init(std::time(nullptr));
	}

	inline Random::Random(std::uint64_t seed)
	{
		init(seed);
	}

	inline void Random::init(std::uint64_t seed)
	{
		// Algorithm: SplitMix64
		// Source: https://prng.di.unimi.it/splitmix64.c

		for (std::uint64_t& state : m_state)
		{
			std::uint64_t z = (seed += 0x9e3779b97f4a7c15);
			z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
			z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
			state = z ^ (z >> 31);
		}
	}

	inline bool Random::flip()
	{
		return static_cast<bool>(random() & 1);
	}

	inline bool Random::prob(float p)
	{
		return fp32() < p;
	}

	inline float Random::fp32()
	{
		return (random() >> 40) * 0x1.0p-24;
	}

	inline std::uint64_t Random::random()
	{
		// Algorithm: xoshiro256++
		// Source: https://prng.di.unimi.it/xoshiro256plusplus.c

		const std::uint64_t result = rotl(m_state[0] + m_state[3], 23) + m_state[0];
		const std::uint64_t t = m_state[1] << 17;

		m_state[2] ^= m_state[0];
		m_state[3] ^= m_state[1];
		m_state[1] ^= m_state[2];
		m_state[0] ^= m_state[3];

		m_state[2] ^= t;
		m_state[3] = rotl(m_state[3], 45);

		return result;
	}

	inline std::uint32_t Random::rd32(std::uint64_t b)
	{
		assert(b < (1ULL << 32));
		const std::uint64_t r = random() & 0xFFFFFFFF;
		return static_cast<std::uint32_t>((r * b) >> 32);
	}

	inline std::pair<std::uint32_t, std::uint32_t> Random::rd32x2(std::uint64_t b1, std::uint64_t b2)
	{
		assert(b1 < (1ULL << 32));
		assert(b2 < (1ULL << 32));

		const std::uint64_t r = random();
		const std::uint64_t r1 = r & 0xFFFFFFFF;
		const std::uint64_t r2 = r >> 32;

		return { static_cast<std::uint32_t>((r1 * b1) >> 32), static_cast<std::uint32_t>((r2 * b2) >> 32) };
	}
}
