#include "pch.hpp"
#include "rand.hpp"

Random::Random() : bValueCount(0), randValue(0) { Init(); }

bool Random::GetBool()
{
	const bool out = bool(randValue & 1);

	if (bValueCount > 1)
	{
		randValue >>= 1;
		bValueCount--;
	}
	else
	{
		randValue = engine();
		bValueCount = sizeof(var_type) * 8;
	}

	return out;
}

float Random::GetFloat()
{
	return engine() / float(std::numeric_limits<var_type>::max());
}

double Random::GetDouble()
{
	return engine() / double(std::numeric_limits<var_type>::max());
}

auto Random::operator()() -> var_type { return engine(); }
auto Random::GetEngine() -> engine_type& { return engine; }

void Random::Init(const var_type seed)
{
	engine.seed(seed);
	randValue = engine();
	bValueCount = sizeof(var_type) * 8;
}