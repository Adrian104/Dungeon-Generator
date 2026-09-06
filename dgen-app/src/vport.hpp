// SPDX-FileCopyrightText: Copyright (c) 2026 Adrian Kulawik
// SPDX-License-Identifier: MIT

#pragma once
#include "pch.hpp"

template <typename T>
class ViewportBase
{
protected:
	T m_scale = static_cast<T>(1);
	T m_xdisp = static_cast<T>(0);
	T m_ydisp = static_cast<T>(0);

public:
	T get_scale() const;
	std::pair<T, T> get_displacement() const;

	template <typename U, typename V = U>
	V to_world(const U& screen) const;

	template <typename U, typename V = U>
	V to_screen(const U& world) const;
};

class Viewport : public ViewportBase<float>
{
private:
	float m_xstart = 0;
	float m_ystart = 0;
	float m_defScale = 1.0f;
	float m_scaleStep = 0.2f;
	bool m_pressed = false;

	void move(float xmouse, float ymouse);
	void scale(float xmouse, float ymouse, float factor);

public:
	void reset();
	bool update(SDL_Event& event);
	void set_scale_step(float step);
	void set_default_scale(float scale);
};

template <typename T>
inline T ViewportBase<T>::get_scale() const
{
	return m_scale;
}

template <typename T>
inline std::pair<T, T> ViewportBase<T>::get_displacement() const
{
	return { m_xdisp, m_ydisp };
}

template <typename T> template <typename U, typename V>
inline V ViewportBase<T>::to_world(const U& screen) const
{
	V world;

	constexpr static bool s_point =
		(std::is_same_v<U, SDL_Point> || std::is_same_v<U, SDL_FPoint> || std::is_same_v<U, dg::Vec>) &&
		(std::is_same_v<V, SDL_Point> || std::is_same_v<V, SDL_FPoint> || std::is_same_v<V, dg::Vec>);

	constexpr static bool s_rect =
		(std::is_same_v<U, SDL_Rect> || std::is_same_v<U, SDL_FRect> || std::is_same_v<U, dg::Rect>) &&
		(std::is_same_v<V, SDL_Rect> || std::is_same_v<V, SDL_FRect> || std::is_same_v<V, dg::Rect>);

	if constexpr (s_point || s_rect)
	{
		world.x = static_cast<decltype(V::x)>(screen.x / m_scale + m_xdisp);
		world.y = static_cast<decltype(V::y)>(screen.y / m_scale + m_ydisp);
	}

	if constexpr (s_rect)
	{
		world.w = static_cast<decltype(V::w)>(screen.w / m_scale);
		world.h = static_cast<decltype(V::h)>(screen.h / m_scale);
	}

	if constexpr (!s_point && !s_rect)
	{
		static_assert(!sizeof(V*));
	}

	return world;
}

template <typename T> template <typename U, typename V>
inline V ViewportBase<T>::to_screen(const U& world) const
{
	V screen;

	constexpr static bool s_point =
		(std::is_same_v<U, SDL_Point> || std::is_same_v<U, SDL_FPoint> || std::is_same_v<U, dg::Vec>) &&
		(std::is_same_v<V, SDL_Point> || std::is_same_v<V, SDL_FPoint> || std::is_same_v<V, dg::Vec>);

	constexpr static bool s_rect =
		(std::is_same_v<U, SDL_Rect> || std::is_same_v<U, SDL_FRect> || std::is_same_v<U, dg::Rect>) &&
		(std::is_same_v<V, SDL_Rect> || std::is_same_v<V, SDL_FRect> || std::is_same_v<V, dg::Rect>);

	if constexpr (s_point || s_rect)
	{
		screen.x = static_cast<decltype(V::x)>((world.x - m_xdisp) * m_scale);
		screen.y = static_cast<decltype(V::y)>((world.y - m_ydisp) * m_scale);
	}

	if constexpr (s_rect)
	{
		screen.w = static_cast<decltype(V::w)>(world.w * m_scale);
		screen.h = static_cast<decltype(V::h)>(world.h * m_scale);
	}

	if constexpr (!s_point && !s_rect)
	{
		static_assert(!sizeof(V*));
	}

	return screen;
}
