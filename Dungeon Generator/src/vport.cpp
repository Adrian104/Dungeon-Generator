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
	else if (sdlEvent.type == SDL_KEYDOWN)
	{
		if (sdlEvent.key.keysym.sym == SDLK_F1)
		{
			scale = 1;
			xOffset = 0;
			yOffset = 0;
		}
	}
}

void Viewport::Scale(float wFrom, float hFrom, int &wTo, int &hTo) const
{
	wTo = int(wFrom * scale);
	hTo = int(hFrom * scale);
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

void Viewport::RectToScreen(float xWorld, float yWorld, float width, float height, SDL_Rect &rect)
{
	ToScreen(xWorld, yWorld, rect.x, rect.y);
	Scale(width, height, rect.w, rect.h);
}