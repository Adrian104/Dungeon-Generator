#pragma once

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
using uint = unsigned int;
using byte = unsigned char;

inline Vec operator+(const Point &p1, const Point &p2) { return Vec(p1.x + p2.x, p1.y + p2.y); }
inline Vec operator-(const Point &p1, const Point &p2) { return Vec(p1.x - p2.x, p1.y - p2.y); }

enum Dir : byte { NORTH, EAST, SOUTH, WEST, INVALID };