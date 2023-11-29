// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#include "benchmark.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>

template <typename Type>
static size_t SizeInBytes(const Type& container)
{
	return container.size() * sizeof(typename Type::value_type);
}

void Probe::Start()
{
	m_hash = g_initialHash;
	m_timePoints.clear();
	m_startTimePoint = clock_type::now();
}

void Probe::Measure(const char* name)
{
	m_timePoints.emplace_back(clock_type::now(), name);
}

void Probe::ComputeHash(const void* data, size_t size)
{
	const hash_type* crr = static_cast<const hash_type*>(data);
	const hash_type* const end = static_cast<const hash_type*>(data) + (size / sizeof(hash_type));

	while (crr != end)
	{
		m_hash ^= *(crr++);
		m_hash ^= m_hash << 13;
		m_hash ^= m_hash >> 7;
		m_hash ^= m_hash << 17;
	}

	const uint8_t* crr8 = reinterpret_cast<const uint8_t*>(crr);
	const uint8_t* const end8 = static_cast<const uint8_t*>(data) + size;

	while (crr8 != end8)
	{
		m_hash ^= *(crr8++);
		m_hash ^= m_hash << 13;
		m_hash ^= m_hash >> 7;
		m_hash ^= m_hash << 17;
	}
}

void TimedGenerator::TimedGenerate(Probe& probe, const dg::Input* input)
{
	dg::Output output;

	probe.Start();
	m_input = input;
	m_output = &output;

	Prepare();
	probe.Measure("Prepare");

	GenerateTree(*m_rootNode, m_input->m_maxDepth);
	probe.Measure("Generate Tree");

	GenerateRooms();
	probe.Measure("Generate Rooms");

	CreateVertices();
	probe.Measure("Create Vertices");

	FindPaths();
	probe.Measure("Find Paths");

	OptimizeVertices();
	probe.Measure("Optimize Vertices");

	GenerateOutput();
	probe.Measure("Generate Output");

	Clear();
	probe.Measure("Clear");

	probe.ComputeHash(output.m_rooms.data(), SizeInBytes(output.m_rooms));
	probe.ComputeHash(output.m_paths.data(), SizeInBytes(output.m_paths));
	probe.ComputeHash(output.m_entrances.data(), SizeInBytes(output.m_entrances));
}

void Config::Load(ini::section_type& section, bool req)
{
	ini::Get(section, "minIter", m_minIter, req);
	ini::Get(section, "minTime", m_minTime, req);
	ini::Get(section, "minWarmupIter", m_minWarmupIter, req);
	ini::Get(section, "minWarmupTime", m_minWarmupTime, req);

	ini::Get(section, "seed", m_seed, req);
	ini::Get(section, "width", m_width, req);
	ini::Get(section, "height", m_height, req);
	ini::Get(section, "minDepth", m_minDepth, req);
	ini::Get(section, "maxDepth", m_maxDepth, req);
	ini::Get(section, "minRoomSize", m_minRoomSize, req);
	ini::Get(section, "maxRoomSize", m_maxRoomSize, req);
	ini::Get(section, "pathCostFactor", m_pathCostFactor, req);
	ini::Get(section, "extraPathCount", m_extraPathCount, req);
	ini::Get(section, "extraPathDepth", m_extraPathDepth, req);
	ini::Get(section, "sparseAreaDens", m_sparseAreaDens, req);
	ini::Get(section, "sparseAreaProb", m_sparseAreaProb, req);
	ini::Get(section, "sparseAreaDepth", m_sparseAreaDepth, req);
	ini::Get(section, "doubleRoomProb", m_doubleRoomProb, req);
	ini::Get(section, "heuristicFactor", m_heuristicFactor, req);
	ini::Get(section, "spaceInterdistance", m_spaceInterdistance, req);
	ini::Get(section, "generateFewerPaths", m_generateFewerPaths, req);
	ini::Get(section, "spaceSizeRandomness", m_spaceSizeRandomness, req);
}

void Trial::Interpret(const Probe& probe, std::vector<const char*>& columns, bool measure)
{
	if (columns.empty())
	{
		columns.reserve(probe.m_timePoints.size());
		for (const auto& [time, name] : probe.m_timePoints)
			columns.push_back(name);
	}

	const size_t size = columns.size();
	if (probe.m_timePoints.size() != size)
		throw std::runtime_error("Unsteady measurement");

	if (m_durations.empty())
		m_durations.resize(size);

	if (measure)
	{
		duration_type diff, total = duration_type::zero();
		time_type prevTime = probe.m_startTimePoint;

		for (size_t i = 0; i < size; i++)
		{
			const auto& [time, name] = probe.m_timePoints.at(i);
			if (name != columns.at(i))
				throw std::runtime_error("Unsteady measurement");

			diff = time - prevTime;
			m_durations[i].m_sum += diff;

			total += diff;
			prevTime = time;
		}

		m_sumDuration += total;
		if (m_minDuration > total)
		{
			m_minDuration = total;
			prevTime = probe.m_startTimePoint;

			for (size_t i = 0; i < size; i++)
			{
				const time_type time = probe.m_timePoints.at(i).first;
				m_durations[i].m_min = time - prevTime;
				prevTime = time;
			}
		}

		m_iterations++;
	}

	if (m_hashBehavior == HashBehavior::STEADY)
	{
		if (m_hash != probe.m_hash)
			m_hashBehavior = HashBehavior::UNSTEADY;
	}
	else if (m_hashBehavior == HashBehavior::UNKNOWN)
	{
		m_hash = probe.m_hash;
		m_hashBehavior = HashBehavior::STEADY;
	}
}

void Benchmark::RunTrials()
{
	using std::chrono::milliseconds;

	if (m_delay > 0)
	{
		std::cout << "\n Delay " << m_delay << " ms\n" << std::flush;
		std::this_thread::sleep_for(milliseconds(m_delay));
	}

	std::cout << "\n ************ BENCHMARKING ************\n\n";

	Probe probe;
	unsigned int index = 1;

	for (Trial& trial : m_trials)
	{
		std::cout << ' ' << index++ << '/' << m_trials.size() << ' ' << trial.m_name << std::flush;

		bool measure = false;
		int remIter = trial.m_config.m_minWarmupIter;

		time_type now;
		time_type startTime = clock_type::now();
		duration_type minDuration = milliseconds(trial.m_config.m_minWarmupTime);

		while (true)
		{
			TimedGenerator generator;
			generator.TimedGenerate(probe, &trial.m_config);
			trial.Interpret(probe, m_columns, measure);

			remIter--;
			now = clock_type::now();

			if (remIter > 0 || minDuration > now - startTime)
				continue;

			if (measure)
				break;

			measure = true;
			remIter = trial.m_config.m_minIter;

			startTime = now;
			minDuration = milliseconds(trial.m_config.m_minTime);

			std::cout << "... " << std::flush;
		}

		const auto elapsed = std::chrono::duration_cast<milliseconds>(now - startTime).count();
		std::cout << "done! (" << elapsed << " ms, " << trial.m_iterations << " i)\n" << std::flush;
	}

	std::cout << '\n';
}

void Benchmark::LoadConfig()
{
	ini::container_type container;
	ini::Load(container, "config.ini");
	ini::section_type& globalSection = container[""];

	Config globalConfig{};
	globalConfig.Load(globalSection, true);

	ini::Get(globalSection, "delay", m_delay, true);
	ini::Get(globalSection, "outputFile", m_outputFile, true);

	for (unsigned int i = 0; true; i++)
	{
		const std::string name = "trial_" + std::to_string(i);
		const auto iter = container.find(name);

		if (iter == container.end())
			break;

		ini::section_type& crrSection = iter->second;
		Trial& trial = m_trials.emplace_back(globalConfig, name);

		trial.m_config.Load(crrSection, false);
		ini::Get(crrSection, "name", trial.m_name, false);
	}
}

void Benchmark::SaveSummary() const
{
	std::ofstream file(m_outputFile);
	if (!file.good()) return;

	file << "Name";

	for (const char* column : m_columns)
		file << ',' << column;
	file << ",Avg Total Time";

	for (const char* column : m_columns)
		file << ',' << column;
	file << ",Min Total Time,Iterations,Hash\n";

	for (const Trial& trial : m_trials)
	{
		file << trial.m_name;

		for (const Trial::Durations& durations : trial.m_durations)
			file << ',' << (durations.m_sum / trial.m_iterations).count();
		file << ',' << (trial.m_sumDuration / trial.m_iterations).count();

		for (const Trial::Durations& durations : trial.m_durations)
			file << ',' << durations.m_min.count();
		file << ',' << trial.m_minDuration.count() << ',' << trial.m_iterations << ',';

		switch (trial.m_hashBehavior)
		{
		case HashBehavior::STEADY:
			file << std::hex << std::uppercase;
			file << trial.m_hash;
			file << std::dec << std::nouppercase;
			break;

		case HashBehavior::UNSTEADY:
			file << "Unsteady";
			break;

		default:
			file << "Unknown";
		}

		file << '\n';
	}
}

void Benchmark::Run()
{
	LoadConfig();
	RunTrials();
	SaveSummary();
}