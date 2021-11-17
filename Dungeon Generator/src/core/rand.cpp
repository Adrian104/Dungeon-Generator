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
	static const std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
	return uniform(engine);
}

double Random::GetDouble()
{
	static const std::uniform_real_distribution<double> uniform(0.0, 1.0);
	return uniform(engine);
}

auto Random::operator()() -> var_type { return engine(); }
auto Random::GetEngine() -> engine_type& { return engine; }

void Random::Init(const var_type seed)
{
	engine.seed(seed);
	randValue = engine();
	bValueCount = sizeof(var_type) * 8;
}