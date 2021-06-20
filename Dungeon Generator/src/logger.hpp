#pragma once
#include "global.hpp"

namespace logger
{
	void Stop();
	void LogEndl();
	void LogMsg(const std::string &str);
	void LogTime(const std::string &str);
	void LogHeader(const std::string &str);
	void LogTotalTime(const std::string &str);
}

#ifdef LOGGER_ENABLED
	#define STOP_TIMING() logger::Stop()
	#define LOG_ENDL() logger::LogEndl()
	#define LOG_MSG(str) logger::LogMsg(str)
	#define LOG_TIME(str) logger::LogTime(str)
	#define LOG_HEADER(str) logger::LogHeader(str)
	#define LOG_TOTAL_TIME(str) logger::LogTotalTime(str)
#endif