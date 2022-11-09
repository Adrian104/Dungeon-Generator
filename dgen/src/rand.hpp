#pragma once
#include <random>

class Random
{
	public:
	using engine_type = std::mt19937;
	using result_type = engine_type::result_type;

	private:
	int m_bitCount = 0;
	result_type m_bits = 0;
	engine_type m_engine;

	public:
	Random() { Init(); }
	Random(const result_type seed) { Init(seed); }

	bool GetBool();
	float GetFloat();
	double GetDouble();

	engine_type& GetEngine() { return m_engine; }
	result_type operator()() { return m_engine(); }

	template <typename Type>
	auto operator()(Type&& distribution) { return distribution(m_engine); }

	void Init(const result_type seed = engine_type::default_seed);
};

inline bool Random::GetBool()
{
	if (m_bitCount > 0)
	{
		m_bits >>= 1;
		m_bitCount--;
	}
	else
	{
		m_bits = m_engine();
		m_bitCount = sizeof(result_type) * 8 - 1;
	}

	return static_cast<bool>(m_bits & 0b1);
}

inline float Random::GetFloat()
{
	static constexpr float invMax = 1.0f / static_cast<float>(engine_type::max());
	return m_engine() * invMax;
}

inline double Random::GetDouble()
{
	static constexpr double invMax = 1.0 / static_cast<double>(engine_type::max());
	return m_engine() * invMax;
}

inline void Random::Init(const result_type seed)
{
	m_bitCount = 0;
	m_engine.seed(seed);
}