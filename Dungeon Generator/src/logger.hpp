#pragma once
#include <string>

namespace logger
{
	void Stop();
	void LogEndl();
	void Log(const std::string &str);
	void LogTime(const std::string &str);
	void LogHeader(const std::string &str);
}