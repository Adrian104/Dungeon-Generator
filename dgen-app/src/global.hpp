#pragma once
#include <string>
#include <utility>

inline const int gDefMinDepth = 9;
inline const int gDefMaxDepth = 10;
inline const int gDefRandAreaDepth = 2;
inline const int gDefAdditionalConnections = 2;

inline const float gDefFactor = 1.0f;
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

inline const float gGridMinimumScale = 5.0f;

inline const std::string gTitle = "Dungeon Generator";
inline const std::pair<std::string, int> gFonts[] = { std::pair<std::string, int>("res/Quicksand-Regular.ttf", 16) };