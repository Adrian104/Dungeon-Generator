#pragma once
#include "global.hpp"

template <typename Type>
class ViewportBase
{
	protected:
	Type m_scale = 1;
	Type m_xOffset = 0;
	Type m_yOffset = 0;

	public:
	Type GetScale() const;
	Type GetXOffset() const;
	Type GetYOffset() const;

	template <typename RType>
	void RectToScreen(const RType& p_from, RType& p_to) const;

	template <typename SType, typename WType>
	void ToWorld(SType p_xScreen, SType p_yScreen, WType& p_xWorld, WType& p_yWorld) const;

	template <typename WType, typename SType>
	void ToScreen(WType p_xWorld, WType p_yWorld, SType& p_xScreen, SType& p_yScreen) const;
};

class Viewport : public ViewportBase<float>
{
	int m_xStart = 0;
	int m_yStart = 0;
	bool m_pressed = false;
	float m_defScale = 1.0f;
	float m_scaleStep = 0.25f;

	void Move(int p_xMouse, int p_yMouse);
	void Scale(int p_xMouse, int p_yMouse, float p_factor);

	public:
	bool Update(SDL_Event& p_sdlEvent);

	void Reset();
	void SetScaleStep(float p_scaleStep);
	void SetDefaultScale(float p_defScale);
};

template <typename Type>
inline Type ViewportBase<Type>::GetScale() const
{
	return m_scale;
}

template <typename Type>
inline Type ViewportBase<Type>::GetXOffset() const
{
	return m_xOffset;
}

template <typename Type>
inline Type ViewportBase<Type>::GetYOffset() const
{
	return m_yOffset;
}

template <typename Type> template <typename RType>
void ViewportBase<Type>::RectToScreen(const RType& p_from, RType& p_to) const
{
	ToScreen(p_from.x, p_from.y, p_to.x, p_to.y);

	p_to.w = static_cast<decltype(p_to.w)>(p_from.w * m_scale);
	p_to.h = static_cast<decltype(p_to.h)>(p_from.h * m_scale);
}

template <typename Type> template <typename SType, typename WType>
void ViewportBase<Type>::ToWorld(SType p_xScreen, SType p_yScreen, WType& p_xWorld, WType& p_yWorld) const
{
	p_xWorld = static_cast<WType>(p_xScreen / m_scale + m_xOffset);
	p_yWorld = static_cast<WType>(p_yScreen / m_scale + m_yOffset);
}

template <typename Type> template <typename WType, typename SType>
void ViewportBase<Type>::ToScreen(WType p_xWorld, WType p_yWorld, SType& p_xScreen, SType& p_yScreen) const
{
	p_xScreen = static_cast<SType>((p_xWorld - m_xOffset) * m_scale);
	p_yScreen = static_cast<SType>((p_yWorld - m_yOffset) * m_scale);
}

inline void Viewport::Reset()
{
	m_scale = m_defScale;
	m_xOffset = 0;
	m_yOffset = 0;
}

inline void Viewport::SetScaleStep(float p_scaleStep)
{
	m_scaleStep = p_scaleStep;
}

inline void Viewport::SetDefaultScale(float p_defScale)
{
	if (p_defScale != 0) m_defScale = p_defScale;
}