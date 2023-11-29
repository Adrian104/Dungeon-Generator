// SPDX-FileCopyrightText: Copyright (c) 2023 Adrian Kulawik
// SPDX-License-Identifier: MIT

/**********************************************************
 * This file contains declarations of all public variables,
 * functions and structures. To use dgen library, include
 * only this file in your project.
 *
 * For implementation declarations and definitions, check
 * dgen_impl.hpp and dgen_impl.cpp files respectively.
 *********************************************************/

#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace dg
{
	struct Point
	{
		int x, y;

		Point() = default;
		Point(int pX, int pY) : x(pX), y(pY) {}
	};

	struct Rect
	{
		int x, y, w, h;

		Rect() = default;
		Rect(int pX, int pY, int pW, int pH) : x(pX), y(pY), w(pW), h(pH) {}
	};

	using Vec = Point;

	/// @brief Structure containing the input data for the generator.
	struct Input
	{
		/// @brief Random value used to initialize the internal pseudorandom number generator.
		std::uint64_t m_seed;

		/// @brief Positive integer. Describes width of the map to generate.
		int m_width;

		/// @brief Positive integer. Describes height of the map to generate.
		int m_height;

		/// @brief Positive integer. Describes minimum number of BSP divisions.
		int m_minDepth;

		/// @brief Positive integer. Describes maximum number of BSP divisions.
		int m_maxDepth;

		/// @brief Non-negative integer. Set this to zero, unless you want to produce wide paths later.
		int m_spaceInterdistance;

		/// @brief Value in range [0, 1]. The smaller the value, the more evenly distributed rooms are.
		float m_spaceSizeRandomness;

		/// @brief Non-negative integer. Indicates level from which area might become "sparse".
		int m_sparseAreaDepth;

		/// @brief Value in range [0, 1]. Describes the probability of producing the room in the "sparse" area.
		float m_sparseAreaDens;

		/// @brief Value in range [0, 1]. Describes the probability of marking area as "sparse".
		float m_sparseAreaProb;

		/// @brief Value in range [0, 1]. Describes minimum width and height factor of all rooms.
		float m_minRoomSize;

		/// @brief Value in range [0, 1]. Describes maximum width and height factor of all rooms.
		float m_maxRoomSize;

		/// @brief Value in range [0, 1]. Describes the probability of producing double rooms.
		float m_doubleRoomProb;

		/// @brief Value in range [0, 1]. The higher the value, the additional paths are more likely to be generated.
		float m_heuristicFactor;

		/// @brief Value in range [0, 1]. The higher the value, the additional paths are more likely to be generated.
		float m_pathCostFactor;

		/// @brief Non-negative integer. The higher the value, the more paths are found down to the depth of @ref m_extraPathDepth
		int m_extraPathCount;

		/// @brief Non-negative integer. Indicates level to which generate extra paths.
		int m_extraPathDepth;

		/// @brief If true, generator may produce fewer paths without altering the effective geometry.
		bool m_generateFewerPaths;
	};

	/// @brief Structure containing the output data of the generator.
	struct Output
	{
		/// @brief Each room is described by its coordinates and size.
		std::vector<Rect> m_rooms;

		/// @brief Each entrance is described by its coordinates.
		std::vector<Point> m_entrances;

		/// @brief Each path is described by its starting point and shift vector.
		std::vector<std::pair<Point, Vec>> m_paths;
	};

	/// @brief Function returns example input data, useful for testing.
	/// @return Object that holds input data for the generator.
	Input GetExampleInput();

	/// @brief Generates a dungeon.
	/// @param input Pointer to existing Input structure, already containing input information.
	/// @param output Pointer to existing Output structure. Generated data about dungeon will be stored inside.
	void Generate(const Input* input, Output* output);
}