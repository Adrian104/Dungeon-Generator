#include "rand.hpp"

template <typename Type>
static constexpr unsigned int CountBits(Type var)
{
	unsigned int count = 0;
	while (var != 0)
	{
		var >>= 1;
		count++;
	}

	return count;
}

bool Random::GetBool()
{
	if (m_bitCount > 0)
	{
		m_bits >>= 1;
		m_bitCount--;
	}
	else
	{
		m_bits = m_engine();
		m_bitCount = CountBits(engine_type::max()) - 1;
	}

	return static_cast<bool>(m_bits & 0b1);
}

float Random::GetFloat()
{
	static constexpr float invMax = 1.0f / static_cast<float>(engine_type::max());
	return m_engine() * invMax;
}

double Random::GetDouble()
{
	static constexpr double invMax = 1.0 / static_cast<double>(engine_type::max());
	return m_engine() * invMax;
}

void Random::Init(const seed_type seed)
{
	m_bitCount = 0;
	m_engine.seed(seed);
}