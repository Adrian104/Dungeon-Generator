// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include <chrono>
#include <utility>

extern unsigned char g_fontData[];
extern unsigned int g_fontDataSize;

inline const int g_minDepth = 9;
inline const int g_maxDepth = 10;
inline const int g_extraPathCount = 3;
inline const int g_extraPathDepth = 2;
inline const int g_sparseAreaDepth = 2;
inline const int g_spaceInterdistance = 1;

inline const float g_factor = 1.0f;
inline const float g_minRoomSize = 0.45f;
inline const float g_maxRoomSize = 0.75f;
inline const float g_pathCostFactor = 0.4f;
inline const float g_sparseAreaDens = 0.4f;
inline const float g_sparseAreaProb = 0.2f;
inline const float g_doubleRoomProb = 0.35f;
inline const float g_heuristicFactor = 0.4f;
inline const float g_spaceSizeRandomness = 0.35f;

inline const bool g_visRooms = true;
inline const bool g_visPaths = true;
inline const bool g_visEntrances = false;
inline const bool g_generateFewerPaths = true;

inline const int g_menuMargin = 5;
inline const int g_menuWidth = 300;
inline const int g_menuTitleMargin = 40;
inline const int g_menuOutlineWidth = 3;
inline const int g_menuInternalOffset = 18;
inline const int g_menuExternalOffset = 30;

inline const int g_helpXOffset = 22;
inline const int g_helpYOffset = 8;
inline const int g_helpAnyKeyOffset = 36;

inline const int g_inputWidth = 400;
inline const int g_inputHeight = 100;
inline const int g_inputXOffset1 = 15;
inline const int g_inputXOffset2 = 160;

inline const int g_warningMargin = 20;
inline const float g_gridThresholdScale = 8.0f;

inline const auto g_menuAnimTime = std::chrono::milliseconds(350);
inline const auto g_infoAnimTime = std::chrono::milliseconds(100);
inline const auto g_helpAnimTime = std::chrono::milliseconds(500);
inline const auto g_inputAnimTime = std::chrono::milliseconds(150);
inline const auto g_warningAnimTime = std::chrono::milliseconds(500);

inline const char* const g_title = "Dungeon Generator";
inline const std::pair<const char*, const char*> g_buttonDescriptions[] =
{
	std::pair<const char*, const char*>("G", "Generate map with random seed"),
	std::pair<const char*, const char*>("N", "Generate map with next seed"),
	std::pair<const char*, const char*>("I", "Toggle information window"),
	std::pair<const char*, const char*>("D", "Toggle debug view"),
	std::pair<const char*, const char*>("R", "Reset all settings"),
	std::pair<const char*, const char*>("Tab", "Reset viewport"),
	std::pair<const char*, const char*>("F11", "Toggle fullscreen"),
	std::pair<const char*, const char*>("Enter", "Open input dialog"),
	std::pair<const char*, const char*>("Arrows", "Select/modify variable"),
	std::pair<const char*, const char*>("Esc", "Exit")
};