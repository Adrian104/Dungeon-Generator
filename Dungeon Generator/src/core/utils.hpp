#pragma once
#include "pch.hpp"

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

template <typename RType, typename ...CArgs>
struct Caller
{
	virtual RType Call(CArgs ...args) const = 0;
};

template <typename RType, typename ...CArgs>
struct FCaller : public Caller<RType, CArgs...>
{
	std::function<RType(CArgs...)> m_func;

	FCaller(std::function<RType(CArgs...)> func) : m_func(func) {}
	RType Call(CArgs ...args) const override { return m_func(args...); }
};

template <typename RType, typename Class, typename ...CArgs>
struct MCaller : public Caller<RType, CArgs...>
{
	std::function<RType(Class&, CArgs...)> m_func;
	Class *m_obj;

	MCaller(std::function<RType(Class&, CArgs...)> func, Class *obj) : m_func(func), m_obj(obj) {}
	RType Call(CArgs ...args) const override { return m_func(*m_obj, args...); }
};

using Vec = Point;
using uint = unsigned int;
using byte = unsigned char;

inline Vec operator+(const Point &p1, const Point &p2) { return Vec(p1.x + p2.x, p1.y + p2.y); }
inline Vec operator-(const Point &p1, const Point &p2) { return Vec(p1.x - p2.x, p1.y - p2.y); }

enum Dir : byte { NORTH, EAST, SOUTH, WEST, INVALID };