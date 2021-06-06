#include "pch.hpp"
#include "logger.hpp"

namespace logger
{
	bool isRunning;
	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point stop;

	void Stop()
	{
		if (!isRunning) return;

		stop = std::chrono::steady_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
		double time = diff / 1000.0;

		std::string unit = "ms";
		if (time >= 1000.0) { time /= 1000.0; unit = "s"; }

		time = int(time * 100) / 100.0;
		std::cout << " ( " << time << " " << unit << " )\n";

		isRunning = false;
	}

	void LogEndl()
	{
		Stop();
		std::cout << "\n";
	}

	void Log(const std::string &str)
	{
		Stop();
		std::cout << "   " << str << "\n";
	}

	void LogTime(const std::string &str)
	{
		Stop();
		std::cout << " * " << str << "...";

		isRunning = true;
		start = std::chrono::steady_clock::now();
	}
	
	void LogHeader(const std::string &str)
	{
		Stop();
		std::cout << ">> " << str << "\n\n";
	}
}