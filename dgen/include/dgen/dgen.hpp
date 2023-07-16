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

	struct Input
	{
		std::uint64_t m_seed;
		bool m_generateFewerPaths;

		int m_extraPathCount;
		int m_extraPathDepth;
		int m_sparseAreaDepth;
		int m_width, m_height;
		int m_spaceInterdistance;
		int m_minDepth, m_maxDepth;

		float m_pathCostFactor;
		float m_doubleRoomProb;
		float m_heuristicFactor;
		float m_spaceSizeRandomness;
		float m_minRoomSize, m_maxRoomSize;
		float m_sparseAreaDens, m_sparseAreaProb;
	};

	struct Output
	{
		std::vector<Rect> m_rooms;
		std::vector<Point> m_entrances;
		std::vector<std::pair<Point, Vec>> m_paths;
	};

	Input GetExampleInput();
	void Generate(const Input* input, Output* output);
}