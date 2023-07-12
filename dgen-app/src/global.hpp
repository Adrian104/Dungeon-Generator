#pragma once
#include <chrono>
#include <string>
#include <utility>

inline const int g_minDepth = 9;
inline const int g_maxDepth = 10;
inline const int g_sparseAreaDepth = 2;
inline const int g_spaceInterdistance = 2;

inline const float g_factor = 1.0f;
inline const float g_minRoomSize = 0.25f;
inline const float g_maxRoomSize = 0.75f;
inline const float g_pathCostFactor = 0;
inline const float g_sparseAreaDens = 0.5f;
inline const float g_sparseAreaProb = 0.25f;
inline const float g_doubleRoomProb = 0.25f;
inline const float g_heuristicFactor = 0.75f;
inline const float g_spaceSizeRandomness = 0.35f;

inline const bool g_visRooms = true;
inline const bool g_visPaths = true;
inline const bool g_visEntrances = false;
inline const bool g_generateFewerPaths = true;

inline const int g_menuMargin = 5;
inline const int g_menuWidth = 300;
inline const int g_menuTitleMargin = 45;
inline const int g_menuOutlineWidth = 3;
inline const int g_menuInternalOffset = 20;
inline const int g_menuExternalOffset = 35;

inline const int g_inputWidth = 400;
inline const int g_inputHeight = 100;
inline const int g_inputXOffset1 = 15;
inline const int g_inputXOffset2 = 170;

inline const int g_warningMargin = 20;
inline const float g_gridThresholdScale = 8.0f;

inline const auto g_menuAnimTime = std::chrono::milliseconds(350);
inline const auto g_infoAnimTime = std::chrono::milliseconds(100);
inline const auto g_inputAnimTime = std::chrono::milliseconds(150);
inline const auto g_warningAnimTime = std::chrono::milliseconds(500);

inline const std::string g_title = "Dungeon Generator";
inline const std::pair<int, std::string> g_fonts[] = { std::pair<int, std::string>(16, "res/Quicksand-Regular.ttf") };