#pragma once

#if defined _DEBUG || defined CPU_DEMANDING_MODE
	#define DEBUG_ENABLED
#endif

#ifdef DEBUG_ENABLED
	#define INCREMENTAL_SEED
#else
	#define FULL_SCREEN
#endif

#ifdef CPU_DEMANDING_MODE
	inline const float gDefFactor = 0.05f;
	inline const int gDefMinDepth = 17;
	inline const int gDefMaxDepth = 17;
#else
	inline const float gDefFactor = 1.0f;
	inline const int gDefMinDepth = 9;
	inline const int gDefMaxDepth = 10;
#endif

inline const int gDefRandAreaDepth = 2;
inline const int gDefAdditionalConnections = 2;

inline const float gDefMaxRoomSize = 0.75f;
inline const float gDefMinRoomSize = 0.25f;
inline const float gDefRandAreaDens = 0.5f;
inline const float gDefRandAreaProb = 0.25f;
inline const float gDefDoubleRoomProb = 0.25f;
inline const float gDefHeuristicFactor = 0.75f;
inline const float gDefSpaceSizeRandomness = 0.35f;

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
inline const int gOverlayInternalOffset = 21;
inline const int gOverlayExternalOffset = 36;

inline const std::string gTitle = "Dungeon Generator";
inline const std::pair<std::string, int> gFonts[] = { std::pair<std::string, int>("res/Quicksand-Regular.ttf", 16) };