#pragma once
#include "utils.hpp"

class Random
{
	public:
	typedef std::mt19937 engine_type;
	typedef decltype(engine_type()()) var_type;

	private:
	int bValueCount;
	var_type randValue;
	engine_type engine;

	public:
	Random();

	bool GetBool();
	float GetFloat();
	double GetDouble();

	auto operator()() -> var_type;
	auto GetEngine() -> engine_type&;

	void Init(const var_type seed = engine_type::default_seed);
};