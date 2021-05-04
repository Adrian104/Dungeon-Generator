#pragma once
#include <SDL.h>
#include "global.hpp"

class Viewport
{
	float scale;
	float xOffset;
	float yOffset;
	float defScale;
	float scaleStep;

	public:
	Viewport() : scale(1), xOffset(0), yOffset(0), defScale(1), scaleStep(gDefScaleStep) {}

	void Reset();
	bool Update(SDL_Event &sdlEvent);
	void SetScaleStep(float pScaleStep);
	void SetDefaultScale(float pDefScale);

	void Scale(float wFrom, float hFrom, int &wTo, int &hTo) const;
	void ToWorld(int xScreen, int yScreen, float &xWorld, float &yWorld) const;
	void ToScreen(float xWorld, float yWorld, int &xScreen, int &yScreen) const;

	void Scale(float wFrom, float hFrom, float &wTo, float &hTo) const;
	void ToWorld(float xScreen, float yScreen, float &xWorld, float &yWorld) const;
	void ToScreen(float xWorld, float yWorld, float &xScreen, float &yScreen) const;

	void RectToScreen(SDL_Rect &from, SDL_Rect &to) const;
	void RectToScreen(SDL_FRect &from, SDL_FRect &to) const;

	void RectToScreen(float xWorld, float yWorld, float width, float height, SDL_Rect &rect) const;
	void RectToScreen(float xWorld, float yWorld, float width, float height, SDL_FRect &rect) const;
	
	inline float GetScale() const { return scale; }
	inline float GetXOffset() const { return xOffset; }
	inline float GetYOffset() const { return yOffset; }
};