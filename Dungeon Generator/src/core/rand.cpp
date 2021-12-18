#include "pch.hpp"
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
	const bool output = static_cast<bool>(m_bits & 0b1);

	if (m_bitCount > 1)
	{
		m_bits >>= 1;
		m_bitCount--;
	}
	else
	{
		m_bits = m_engine();
		m_bitCount = CountBits(engine_type::max());
	}

	return output;
}

float Random::GetFloat()
{
	return m_engine() / static_cast<float>(engine_type::max());
}

double Random::GetDouble()
{
	return m_engine() / static_cast<double>(engine_type::max());
}

void Random::Init(const seed_type seed)
{
	m_engine.seed(seed);
	m_bits = m_engine();
	m_bitCount = CountBits(engine_type::max());
}