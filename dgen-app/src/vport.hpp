#pragma once
#include <SDL2/SDL.h>

template <typename Type>
class ViewportBase
{
	protected:
	Type m_scale = 1;
	Type m_xOffset = 0;
	Type m_yOffset = 0;

	public:
	Type GetScale() const { return m_scale; }
	Type GetXOffset() const { return m_xOffset; }
	Type GetYOffset() const { return m_yOffset; }

	template <typename RType>
	void RectToScreen(const RType& from, RType& to) const;

	template <typename SType, typename WType>
	void ToWorld(SType xScreen, SType yScreen, WType& xWorld, WType& yWorld) const;

	template <typename WType, typename SType>
	void ToScreen(WType xWorld, WType yWorld, SType& xScreen, SType& yScreen) const;
};

class Viewport : public ViewportBase<float>
{
	int m_xStart = 0;
	int m_yStart = 0;
	bool m_pressed = false;
	float m_defScale = 1.0f;
	float m_scaleStep = 0.25f;

	void Move(int xMouse, int yMouse);
	void Scale(int xMouse, int yMouse, float factor);

	public:
	bool Update(SDL_Event& sdlEvent);

	void Reset();
	void SetScaleStep(float scaleStep);
	void SetDefaultScale(float defScale);
};

template <typename Type> template <typename RType>
void ViewportBase<Type>::RectToScreen(const RType& from, RType& to) const
{
	ToScreen(from.x, from.y, to.x, to.y);

	to.w = static_cast<decltype(to.w)>(from.w * m_scale);
	to.h = static_cast<decltype(to.h)>(from.h * m_scale);
}

template <typename Type> template <typename SType, typename WType>
void ViewportBase<Type>::ToWorld(SType xScreen, SType yScreen, WType& xWorld, WType& yWorld) const
{
	xWorld = static_cast<WType>(xScreen / m_scale + m_xOffset);
	yWorld = static_cast<WType>(yScreen / m_scale + m_yOffset);
}

template <typename Type> template <typename WType, typename SType>
void ViewportBase<Type>::ToScreen(WType xWorld, WType yWorld, SType& xScreen, SType& yScreen) const
{
	xScreen = static_cast<SType>((xWorld - m_xOffset) * m_scale);
	yScreen = static_cast<SType>((yWorld - m_yOffset) * m_scale);
}

inline void Viewport::Reset()
{
	m_scale = m_defScale;
	m_xOffset = 0;
	m_yOffset = 0;
}

inline void Viewport::SetScaleStep(float scaleStep)
{
	m_scaleStep = scaleStep;
}

inline void Viewport::SetDefaultScale(float defScale)
{
	if (defScale != 0) m_defScale = defScale;
}