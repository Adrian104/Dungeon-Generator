#pragma once

#ifdef _DEBUG
	#define LOGGER_ENABLED
	#define INCREMENTAL_SEED
#else
	#define FULL_SCREEN
	#define RANDOM_SEED
#endif

inline const float gDefFactor = 1;
inline const float gDefScaleStep = 0.25f;

inline const int gDefMinDepth = 9;
inline const int gDefMaxDepth = 10;
inline const int gDefMaxRoomSize = 75;
inline const int gDefMinRoomSize = 25;
inline const int gDefDoubleRoomProb = 25;
inline const int gDefSpaceSizeRandomness = 35;
inline const int gDefAdditionalConnections = 2;

inline const int gDefRandAreaSize = 2;
inline const int gDefRandAreaDens = 50;
inline const int gDefRandAreaProb = 25;

inline const bool gDefRoomsVisibility = true;
inline const bool gDefPathsVisibility = true;
inline const bool gDefEntrancesVisibility = false;

inline const int gOverlayXMargin = 6;
inline const int gOverlayYMargin = 4;
inline const int gOverlayWidth = 300;
inline const int gOverlayAnimTime = 350;
inline const int gOverlayTitleYPos = 40;
inline const int gOverlayOutlineWidth = 3;
inline const int gOverlayTitleYOffset = 50;
inline const int gOverlayRefInternalOffset = 21;
inline const int gOverlayRefExternalOffset = 36;

inline const std::pair<std::string, int> gFonts[] =
{
	std::pair<std::string, int>("res/Quicksand-Regular.ttf", 16)
};