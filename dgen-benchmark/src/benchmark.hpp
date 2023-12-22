// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include "dgen_impl.hpp"
#include "ini_utils.hpp"

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

class Probe
{
	hash_type m_hash = g_initialHash;
	time_type m_startTimePoint;
	time_vec_type m_timePoints;

public:
	void Start();
	void Measure(const char* name);
	void ComputeHash(const void* data, size_t size);

	friend class Trial;
};

class TimedGenerator : protected dg::impl::Generator
{
public:
	void TimedGenerate(Probe& probe, const dg::Input* input);
};

class Config : public dg::Input
{
public:
	int m_minIter;
	int m_minTime;
	int m_minWarmupIter;
	int m_minWarmupTime;

	void Load(ini::section_type& section, bool req);
};

class Trial
{
	struct Durations
	{
		duration_type m_sum = duration_type::zero();
		duration_type m_min = duration_type::max();
	};

	Config m_config;
	std::string m_name;

	size_t m_iterations = 0;
	hash_type m_hash = g_initialHash;
	HashBehavior m_hashBehavior = HashBehavior::UNKNOWN;

	std::vector<Durations> m_durations;
	duration_type m_sumDuration = duration_type::zero();
	duration_type m_minDuration = duration_type::max();

public:
	Trial(const Config& config, const std::string& name) : m_config(config), m_name(name) {}
	void Interpret(const Probe& probe, std::vector<const char*>& columns, bool measure);

	friend class Benchmark;
};

class Benchmark
{
	int m_delay = 0;

	std::string m_outputFile;
	std::vector<Trial> m_trials;
	std::vector<const char*> m_columns;

	void RunTrials();
	void LoadConfig();
	void SaveSummary() const;

public:
	void Run();
};