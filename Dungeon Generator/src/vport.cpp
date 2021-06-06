#include "pch.hpp"
#include "vport.hpp"

void Viewport::Reset()
{
	xOffset = 0;
	yOffset = 0;
	scale = defScale;
}

bool Viewport::Update(SDL_Event &sdlEvent)
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
		if (sdlEvent.key.keysym.sym == SDLK_TAB) Reset();
		else return false;
	}
	else return false;

	return true;
}

void Viewport::SetScaleStep(float pScaleStep)
{
	scaleStep = pScaleStep;
}

void Viewport::SetDefaultScale(float pDefScale)
{
	if (pDefScale != 0) defScale = pDefScale;
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

void Viewport::Scale(float wFrom, float hFrom, float &wTo, float &hTo) const
{
	wTo = wFrom * scale;
	hTo = hFrom * scale;
}

void Viewport::ToWorld(float xScreen, float yScreen, float &xWorld, float &yWorld) const
{
	xWorld = xScreen / scale + xOffset;
	yWorld = yScreen / scale + yOffset;
}

void Viewport::ToScreen(float xWorld, float yWorld, float &xScreen, float &yScreen) const
{
	xScreen = (xWorld - xOffset) * scale;
	yScreen = (yWorld - yOffset) * scale;
}

void Viewport::RectToScreen(SDL_Rect &from, SDL_Rect &to) const
{
	ToScreen(float(from.x), float(from.y), to.x, to.y);
	Scale(float(from.w), float(from.h), to.w, to.h);
}

void Viewport::RectToScreen(SDL_FRect &from, SDL_FRect &to) const
{
	ToScreen(from.x, from.y, to.x, to.y);
	Scale(from.w, from.h, to.w, to.h);
}

void Viewport::RectToScreen(float xWorld, float yWorld, float width, float height, SDL_Rect &rect) const
{
	ToScreen(xWorld, yWorld, rect.x, rect.y);
	Scale(width, height, rect.w, rect.h);
}

void Viewport::RectToScreen(float xWorld, float yWorld, float width, float height, SDL_FRect &rect) const
{
	ToScreen(xWorld, yWorld, rect.x, rect.y);
	Scale(width, height, rect.w, rect.h);
}