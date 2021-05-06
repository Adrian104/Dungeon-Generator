#pragma once
#include <string>

#ifdef _DEBUG
	#define RANDOM_COLORS
	#define LOGGER_ENABLED
	#define INCREMENTAL_SEED
#else
	#define NO_DELAY
	#define FULL_SCREEN
	#define RANDOM_SEED
#endif

inline const float gDefScaleStep = 0.25f;

inline const int gMinRoomWH = 4;
inline const int gMinSpaceWH = 5;

inline const int gDefMinDepth = 9;
inline const int gDefMaxDepth = 10;
inline const int gDefMaxRoomSize = 75;
inline const int gDefMinRoomSize = 25;
inline const int gDefDoubleRoomProb = 25;
inline const int gDefSpaceSizeRandomness = 35;

inline const int gDefNodesVisibilityMode = 0;
inline const int gDefPathsVisibilityMode = 2;
inline const bool gDefSpaceVisibility = true;
inline const bool gDefRoomsVisibility = true;

inline const std::pair<std::string, int> gFonts[] =
{
	std::pair<std::string, int>("res/Quicksand-Regular.ttf", 16)
};

inline const int gFontCount = sizeof(gFonts) / sizeof(*gFonts);