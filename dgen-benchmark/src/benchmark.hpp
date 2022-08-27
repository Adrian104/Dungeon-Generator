#pragma once
#include "dgen.hpp"
#include <chrono>
#include <string>
#include <vector>

using hash_type = uint64_t;
using clock_type = std::chrono::high_resolution_clock;
using duration_type = clock_type::duration;
using time_type = clock_type::time_point;
using time_vec_type = std::vector<std::pair<time_type, const char*>>;

enum class HashBehavior { UNKNOWN, STEADY, UNSTEADY };
inline constexpr hash_type g_initialHash = 0x2937468CA759B489;

class Result
{
	hash_type m_hash = g_initialHash;
	time_type m_startTimePoint;
	time_vec_type m_timePoints;

	public:
	void Start();
	void Measure(const char* name);
	void ComputeHash(const void* data, size_t size);

	friend class Test;
};

class TimedGenerator : protected Generator
{
	Result& m_result;
	GenOutput m_output;

	public:
	TimedGenerator(Result& result) : m_result(result) {}
	void TimedGenerate(const GenInput* genInput);
};

class Test
{
	GenInput m_input;
	std::string m_name;

	unsigned int m_iterations = 0;
	hash_type m_hash = g_initialHash;
	std::vector<duration_type> m_durations;
	HashBehavior m_hashBehavior = HashBehavior::UNKNOWN;

	public:
	Test(const GenInput& input, const std::string& name) : m_input(input), m_name(name) {}
	void Interpret(const Result& result, std::vector<const char*>& columns, bool measure);

	friend class Benchmark;
};

class Benchmark
{
	int m_delay;
	int m_minIter;
	int m_minTime;
	int m_minWarmupIter;
	int m_minWarmupTime;

	std::string m_outputFile;
	std::vector<Test> m_tests;
	std::vector<const char*> m_columns;

	void RunTests();
	void LoadConfig();
	void SaveSummary() const;

	public:
	void Run();
};