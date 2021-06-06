#pragma once

struct Point { int x; int y; };
struct Rect { int x; int y; int w; int h; };

typedef Point Vec;

inline Vec operator+(const Point &p1, const Point &p2) { return Vec{ p1.x + p2.x, p1.y + p2.y }; }
inline Vec operator-(const Point &p1, const Point &p2) { return Vec{ p1.x - p2.x, p1.y - p2.y }; }