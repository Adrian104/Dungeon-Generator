#pragma once
#include <random>
#include <type_traits>

class Random
{
	public:
	using engine_type = std::mt19937;
	using result_type = engine_type::result_type;
	using seed_type = std::remove_const<decltype(engine_type::default_seed)>::type;

	private:
	int m_bitCount = 0;
	result_type m_bits = 0;
	engine_type m_engine;

	public:
	Random() { Init(); }
	Random(const seed_type seed) { Init(seed); }

	bool GetBool();
	float GetFloat();
	double GetDouble();

	engine_type& GetEngine() { return m_engine; }
	result_type operator()() { return m_engine(); }

	template <typename Type>
	auto operator()(Type&& distribution) { return distribution(m_engine); }

	void Init(const seed_type seed = engine_type::default_seed);
};