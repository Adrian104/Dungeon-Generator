#pragma once

#ifdef _DEBUG
	#define RANDOM_COLORS
	#define LOGGER_ENABLED
	#define INCREMENTAL_SEED
#else
	#define NO_DELAY
	#define FULL_SCREEN
	#define RANDOM_SEED
#endif

#ifdef LOGGER_ENABLED
	#include "logger.hpp"

	#define LOGGER_STOP() logger::Stop()
	#define LOGGER_LOG(str) logger::Log(str)
	#define LOGGER_LOG_ENDL() logger::LogEndl()
	#define LOGGER_LOG_TIME(str) logger::LogTime(str)
	#define LOGGER_LOG_HEADER(str) logger::LogHeader(str)
#else
	#define LOGGER_STOP()
	#define LOGGER_LOG(str)
	#define LOGGER_LOG_ENDL()
	#define LOGGER_LOG_TIME(str)
	#define LOGGER_LOG_HEADER(str)
#endif