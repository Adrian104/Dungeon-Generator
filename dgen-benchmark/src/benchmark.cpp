#include "benchmark.hpp"
#include "ini_utils.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>

template <typename Type>
static size_t SizeInBytes(const Type& container)
{
	return container.size() * sizeof(typename Type::value_type);
}

void Result::Start()
{
	m_hash = g_initialHash;
	m_timePoints.clear();
	m_startTimePoint = clock_type::now();
}

void Result::Measure(const char* name)
{
	m_timePoints.emplace_back(clock_type::now(), name);
}

void Result::ComputeHash(const void* data, size_t size)
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

void TimedGenerator::TimedGenerate(const GenInput* genInput)
{
	m_result.Start();
	gInput = genInput;
	gOutput = &m_output;

	Prepare();
	m_result.Measure("Prepare");

	GenerateTree(*root, gInput -> m_maxDepth);
	m_result.Measure("Generate Tree");

	GenerateRooms();
	m_result.Measure("Generate Rooms");

	LinkNodes();
	m_result.Measure("Link Nodes");

	FindPaths();
	m_result.Measure("Find Paths");

	OptimizeNodes();
	m_result.Measure("Optimize Nodes");

	GenerateOutput();
	m_result.Measure("Generate Output");

	Clear();
	m_result.Measure("Clear");

	m_result.ComputeHash(m_output.m_rooms.data(), SizeInBytes(m_output.m_rooms));
	m_result.ComputeHash(m_output.m_paths.data(), SizeInBytes(m_output.m_paths));
	m_result.ComputeHash(m_output.m_entrances.data(), SizeInBytes(m_output.m_entrances));
}

void Test::Interpret(const Result& result, std::vector<const char*>& columns, bool measure)
{
	if (columns.empty())
	{
		columns.reserve(result.m_timePoints.size());
		for (const auto& [time, name] : result.m_timePoints)
			columns.push_back(name);
	}

	const size_t size = columns.size();
	if (result.m_timePoints.size() != size)
	{
		unsteady:
		throw std::runtime_error("Unsteady measurement");
	}

	if (m_durations.empty())
		m_durations.resize(size);

	if (measure)
	{
		time_type prevTime = result.m_startTimePoint;
		for (size_t i = 0; i < size; i++)
		{
			const auto& [time, name] = result.m_timePoints.at(i);
			if (name != columns.at(i)) goto unsteady;

			m_durations[i] += time - prevTime;
			prevTime = time;
		}

		m_iterations++;
	}

	if (m_hashBehavior == HashBehavior::STEADY)
	{
		if (m_hash != result.m_hash)
			m_hashBehavior = HashBehavior::UNSTEADY;
	}
	else if (m_hashBehavior == HashBehavior::UNKNOWN)
	{
		m_hash = result.m_hash;
		m_hashBehavior = HashBehavior::STEADY;
	}
}

void Benchmark::RunTests()
{
	using std::chrono::milliseconds;

	if (m_delay > 0)
	{
		std::cout << "\n Delay " << m_delay << " ms\n" << std::flush;
		std::this_thread::sleep_for(milliseconds(m_delay));
	}

	std::cout << "\n ************ BENCHMARKING ************\n\n";

	Result result;
	unsigned int index = 1;

	for (Test& test : m_tests)
	{
		std::cout << ' ' << index++ << '/' << m_tests.size() << ' ' << test.m_name << std::flush;

		bool measure = false;
		int remIter = m_minWarmupIter;

		time_type now;
		time_type startTime = clock_type::now();
		duration_type minDuration = milliseconds(m_minWarmupTime);

		while (true)
		{
			TimedGenerator generator(result);
			generator.TimedGenerate(&test.m_input);
			test.Interpret(result, m_columns, measure);

			remIter--;
			now = clock_type::now();

			if (remIter > 0 || minDuration > now - startTime)
				continue;

			if (measure)
				break;

			measure = true;
			remIter = m_minIter;

			startTime = now;
			minDuration = milliseconds(m_minTime);

			std::cout << "... " << std::flush;
		}

		const auto elapsed = std::chrono::duration_cast<milliseconds>(now - startTime).count();
		std::cout << "Done! (" << elapsed << " ms, " << test.m_iterations << " i)\n" << std::flush;
	}

	std::cout << '\n';
}

void Benchmark::LoadConfig()
{
	auto LoadGenInput = [](ini::section_type& section, GenInput& dest, bool req) -> void
	{
		ini::Get(section, "seed", dest.m_seed, req);
		ini::Get(section, "width", dest.m_width, req);
		ini::Get(section, "height", dest.m_height, req);
		ini::Get(section, "minDepth", dest.m_minDepth, req);
		ini::Get(section, "maxDepth", dest.m_maxDepth, req);
		ini::Get(section, "minRoomSize", dest.m_minRoomSize, req);
		ini::Get(section, "maxRoomSize", dest.m_maxRoomSize, req);
		ini::Get(section, "randAreaDens", dest.m_randAreaDens, req);
		ini::Get(section, "randAreaProb", dest.m_randAreaProb, req);
		ini::Get(section, "randAreaDepth", dest.m_randAreaDepth, req);
		ini::Get(section, "doubleRoomProb", dest.m_doubleRoomProb, req);
		ini::Get(section, "heuristicFactor", dest.m_heuristicFactor, req);
		ini::Get(section, "spaceInterdistance", dest.m_spaceInterdistance, req);
		ini::Get(section, "generateFewerPaths", dest.m_generateFewerPaths, req);
		ini::Get(section, "spaceSizeRandomness", dest.m_spaceSizeRandomness, req);
	};

	ini::container_type container;
	ini::Load(container, "config.ini");
	ini::section_type& globalSection = container[""];

	GenInput globalGenInput;
	LoadGenInput(globalSection, globalGenInput, true);

	ini::Get(globalSection, "delay", m_delay, true);
	ini::Get(globalSection, "minIter", m_minIter, true);
	ini::Get(globalSection, "minTime", m_minTime, true);
	ini::Get(globalSection, "minWarmupIter", m_minWarmupIter, true);
	ini::Get(globalSection, "minWarmupTime", m_minWarmupTime, true);
	ini::Get(globalSection, "outputFile", m_outputFile, true);

	for (unsigned int i = 0; true; i++)
	{
		const std::string name = "test_" + std::to_string(i);
		const auto iter = container.find(name);

		if (iter == container.end())
			break;

		ini::section_type& crrSection = iter -> second;
		Test& crrTest = m_tests.emplace_back(globalGenInput, name);

		LoadGenInput(crrSection, crrTest.m_input, false);
		ini::Get(crrSection, "name", crrTest.m_name, false);
	}
}

void Benchmark::SaveSummary() const
{
	std::ofstream file(m_outputFile);
	if (!file.good()) return;

	file << "Name";
	for (const char* column : m_columns)
		file << '\t' << column;

	file << "\tAvg Time\tIterations\tHash\n";
	for (const Test& test : m_tests)
	{
		file << test.m_name;

		duration_type total = duration_type::zero();
		for (const duration_type duration : test.m_durations)
		{
			total += duration;
			file << '\t' << (duration / test.m_iterations).count();
		}

		file << '\t' << (total / test.m_iterations).count();
		file << '\t' << test.m_iterations << '\t';

		switch (test.m_hashBehavior)
		{
		case HashBehavior::STEADY:
			file << std::hex << std::uppercase;
			file << test.m_hash;
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
	RunTests();
	SaveSummary();
}