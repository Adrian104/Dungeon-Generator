#pragma once
#include "pch.hpp"

struct Point { int x; int y; };
struct Rect { int x; int y; int w; int h; };

typedef Point Vec;
typedef unsigned char byte;

enum Dir : byte { NORTH, EAST, SOUTH, WEST, INVALID };

inline Vec operator+(const Point &p1, const Point &p2) { return Vec{ p1.x + p2.x, p1.y + p2.y }; }
inline Vec operator-(const Point &p1, const Point &p2) { return Vec{ p1.x - p2.x, p1.y - p2.y }; }

template <typename RType, typename ...CArgs>
struct Caller
{
	virtual RType Call(CArgs ...args) const = 0;
};

template <typename RType, typename ...CArgs>
struct FCaller : public Caller<RType, CArgs...>
{
	std::function<RType(CArgs...)> func;

	FCaller(std::function<RType(CArgs...)> pFunc) : func(pFunc) {}
	RType Call(CArgs ...args) const override { return func(args...); }
};

template <typename RType, typename Class, typename ...CArgs>
struct MCaller : public Caller<RType, CArgs...>
{
	std::function<RType(Class&, CArgs...)> func;
	Class *obj;

	MCaller(std::function<RType(Class&, CArgs...)> pFunc, Class *pObj) : func(pFunc), obj(pObj) {}
	RType Call(CArgs ...args) const override { return func(*obj, args...); }
};