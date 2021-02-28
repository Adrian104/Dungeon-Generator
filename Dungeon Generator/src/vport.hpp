#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

class Viewport
{
	float scale;
	float xOffset;
	float yOffset;
	float scaleStep;

	public:
	Viewport(const float pScaleStep) : scale(1), xOffset(0), yOffset(0), scaleStep(pScaleStep) {}

	float Scale(float toScale);
	void Update(SDL_Event &sdlEvent);

	void Scale(float wFrom, float hFrom, int &wTo, int &hTo) const;
	void ToWorld(int xScreen, int yScreen, float &xWorld, float &yWorld) const;
	void ToScreen(float xWorld, float yWorld, int &xScreen, int &yScreen) const;

	void RectToScreen(SDL_Rect &from, SDL_Rect &to) const;
	void RectToScreen(float xWorld, float yWorld, float width, float height, SDL_Rect &rect) const;
};