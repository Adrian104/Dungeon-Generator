#pragma once

class Random
{
	public:
	typedef std::mt19937 engine_type;
	typedef engine_type::result_type result_type;
	typedef std::remove_const<decltype(engine_type::default_seed)>::type seed_type;

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

	auto operator()() -> result_type;
	auto GetEngine() -> engine_type&;

	void Init(const seed_type seed = engine_type::default_seed);
};

inline auto Random::operator()() -> result_type
{
	return m_engine();
}

inline auto Random::GetEngine() -> engine_type&
{
	return m_engine;
}