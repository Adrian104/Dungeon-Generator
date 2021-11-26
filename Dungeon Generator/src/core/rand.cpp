#include "pch.hpp"
#include "rand.hpp"

template <typename Type>
constexpr uint CountBits(Type var)
{
	uint c = 0;
	while (var != 0)
	{
		var >>= 1;
		c++;
	}

	return c;
}

Random::Random() : bValueCount(0), bValues(0) { Init(); }

bool Random::GetBool()
{
	const bool out = bool(bValues & 0b1);

	if (bValueCount > 1)
	{
		bValues >>= 1;
		bValueCount--;
	}
	else
	{
		bValues = engine();
		bValueCount = CountBits(engine_type::max());
	}

	return out;
}

float Random::GetFloat() { return engine() / float(engine_type::max()); }
double Random::GetDouble() { return engine() / double(engine_type::max()); }

auto Random::operator()() -> result_type { return engine(); }
auto Random::GetEngine() -> engine_type& { return engine; }

void Random::Init(const seed_type seed)
{
	engine.seed(seed);
	bValues = engine();
	bValueCount = CountBits(engine_type::max());
}