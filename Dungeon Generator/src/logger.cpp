#include "pch.hpp"
#include "logger.hpp"

namespace logger
{
	bool isRunning;
	std::chrono::steady_clock::time_point last;
	std::chrono::steady_clock::time_point start;

	void Stop()
	{
		if (!isRunning) return;

		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count();
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

	void LogMsg(const std::string &str)
	{
		Stop();
		std::cout << "   " << str << "\n";
	}

	void LogTime(const std::string &str)
	{
		if (!isRunning) start = std::chrono::steady_clock::now();

		Stop();
		std::cout << " * " << str << "...";

		isRunning = true;
		last = std::chrono::steady_clock::now();
	}

	void LogHeader(const std::string &str)
	{
		Stop();
		std::cout << ">> " << str << "\n\n";
	}

	void LogTotalTime(const std::string &str)
	{
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
		double time = diff / 1000.0;

		std::string unit = "ms";
		if (time >= 1000.0) { time /= 1000.0; unit = "s"; }

		time = int(time * 100) / 100.0;
		std::cout << "   " << str << time << " " << unit << "\n";
	}
}