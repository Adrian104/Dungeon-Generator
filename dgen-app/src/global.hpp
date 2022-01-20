#pragma once
#include <string>
#include <utility>

inline const int g_minDepth = 9;
inline const int g_maxDepth = 10;
inline const int g_randAreaDepth = 2;
inline const int g_additionalConnections = 2;

inline const float g_factor = 1.0f;
inline const float g_maxRoomSize = 0.75f;
inline const float g_minRoomSize = 0.25f;
inline const float g_randAreaDens = 0.5f;
inline const float g_randAreaProb = 0.25f;
inline const float g_doubleRoomProb = 0.25f;
inline const float g_heuristicFactor = 0.75f;
inline const float g_spaceSizeRandomness = 0.35f;

inline const bool g_roomsVisibility = true;
inline const bool g_pathsVisibility = true;
inline const bool g_entrancesVisibility = false;

inline const int g_ovMargin = 5;
inline const int g_ovWidth = 300;
inline const int g_ovAnimTime = 350;
inline const int g_ovTitleMargin = 45;
inline const int g_ovOutlineWidth = 3;
inline const int g_ovInternalOffset = 21;
inline const int g_ovExternalOffset = 36;

inline const float g_gridMinimumScale = 5.0f;

inline const std::string g_title = "Dungeon Generator";
inline const std::pair<std::string, int> g_fonts[] = { std::pair<std::string, int>("res/Quicksand-Regular.ttf", 16) };