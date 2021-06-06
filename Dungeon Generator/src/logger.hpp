#pragma once
#include "global.hpp"

namespace logger
{
	void Stop();
	void LogEndl();
	void Log(const std::string &str);
	void LogTime(const std::string &str);
	void LogHeader(const std::string &str);
}

#ifdef LOGGER_ENABLED
	#define LOGGER_STOP() logger::Stop()
	#define LOGGER_LOG(str) logger::Log(str)
	#define LOGGER_LOG_ENDL() logger::LogEndl()
	#define LOGGER_LOG_TIME(str) logger::LogTime(str)
	#define LOGGER_LOG_HEADER(str) logger::LogHeader(str)
#endif