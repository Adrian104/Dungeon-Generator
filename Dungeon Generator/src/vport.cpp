#include "vport.hpp"

float Viewport::Scale(float toScale)
{
	return toScale * scale;
}

void Viewport::Update(SDL_Event &sdlEvent)
{
	static int xStart = 0;
	static int yStart = 0;
	static bool pressed = false;

	int xTemp, yTemp;
	pressed &= SDL_GetMouseState(&xTemp, &yTemp) & SDL_BUTTON(SDL_BUTTON_LEFT);

	if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
	{
		xStart = xTemp;
		yStart = yTemp;
		pressed = true;
	}
	else if (sdlEvent.type == SDL_MOUSEMOTION && pressed)
	{
		xOffset += (xStart - xTemp) / scale;
		yOffset += (yStart - yTemp) / scale;

		xStart = xTemp;
		yStart = yTemp;
	}
	else if (sdlEvent.type == SDL_MOUSEWHEEL)
	{
		float xBefore, yBefore;
		ToWorld(xTemp, yTemp, xBefore, yBefore);

		if (sdlEvent.wheel.y > 0) scale *= (1 + scaleStep);
		else if (sdlEvent.wheel.y < 0) scale *= (1 - scaleStep);

		float xAfter, yAfter;
		ToWorld(xTemp, yTemp, xAfter, yAfter);

		xOffset += xBefore - xAfter;
		yOffset += yBefore - yAfter;
	}
}

void Viewport::ToWorld(int xScreen, int yScreen, float &xWorld, float &yWorld) const
{
	xWorld = float(xScreen) / scale + xOffset;
	yWorld = float(yScreen) / scale + yOffset;
}

void Viewport::ToScreen(float xWorld, float yWorld, int &xScreen, int &yScreen) const
{
	xScreen = int((xWorld - xOffset) * scale);
	yScreen = int((yWorld - yOffset) * scale);
}