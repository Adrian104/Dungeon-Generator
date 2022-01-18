#include <chrono>
#include "overlay.hpp"

const int gOverlayXMargin2 = gOverlayXMargin << 1;
const int gOverlayYMargin2 = gOverlayYMargin << 1;

Overlay::Overlay(AppManager& appManager) : Animator(std::chrono::milliseconds(gOverlayAnimTime)), selected(0), refresh(true), mgr(appManager), texture(nullptr) {}

Overlay::~Overlay()
{
	for (Modifier *&mod : mods) delete mod;
	DestroyResources();
}

void Overlay::Draw()
{
	const SDL_Rect dest = { int(-gOverlayWidth * GetPhase()), 0, gOverlayWidth, mgr.GetHeight() };
	SDL_RenderCopy(mgr.GetRenderer(), texture, nullptr, &dest);
}

void Overlay::Render()
{
	SDL_Rect rect = { gOverlayWidth - gOverlayOutlineWidth, 0, gOverlayOutlineWidth, mgr.GetHeight() };

	SDL_SetRenderTarget(mgr.GetRenderer(), texture);
	SDL_SetRenderDrawColor(mgr.GetRenderer(), 0, 0, 0, 0xCC);
	SDL_RenderClear(mgr.GetRenderer());

	SDL_SetRenderDrawColor(mgr.GetRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(mgr.GetRenderer(), &rect);

	Text text = mgr.RenderText(gTitle, 0, TTF_STYLE_BOLD);
	int xCenter = XCenter(text);

	rect = { xCenter, gOverlayTitleYPos, text.GetWidth(), text.GetHeight() };
	SDL_RenderCopy(mgr.GetRenderer(), text.GetTexture(), nullptr, &rect);

	rect = { xCenter - gOverlayXMargin, gOverlayTitleYPos - gOverlayYMargin, text.GetWidth() + gOverlayXMargin2, text.GetHeight() + gOverlayYMargin2 };
	SDL_RenderDrawRect(mgr.GetRenderer(), &rect);

	int crrIndex = 0;
	int crrPos = gOverlayTitleYPos + gOverlayTitleYOffset;

	for (Modifier *&mod : mods)
	{
		const uint8_t colorVal = crrIndex == selected ? 0 : 0xFF;

		text = mgr.RenderText(mod -> name, 0, TTF_STYLE_BOLD, { colorVal, 0xFF, colorVal, 0xFF });
		xCenter = XCenter(text);

		rect = { xCenter, crrPos, text.GetWidth(), text.GetHeight() };
		SDL_RenderCopy(mgr.GetRenderer(), text.GetTexture(), nullptr, &rect);
		crrPos += gOverlayInternalOffset;

		text = mgr.RenderText(mod -> GetValue(), 0);
		xCenter = XCenter(text);

		rect = { xCenter, crrPos, text.GetWidth(), text.GetHeight() };
		SDL_RenderCopy(mgr.GetRenderer(), text.GetTexture(), nullptr, &rect);
		crrPos += gOverlayExternalOffset;

		crrIndex++;
	}

	refresh = false;
}

bool Overlay::Update()
{
	if (refresh)
	{
		Render();
		if (IsPlaying()) Animator::Update();

		return true;
	}

	if (IsPlaying())
	{
		Animator::Update();
		return true;
	}

	return false;
}

void Overlay::InitResources()
{
	DestroyResources();

	texture = SDL_CreateTexture(mgr.GetRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gOverlayWidth, mgr.GetHeight());
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
}

void Overlay::DestroyResources()
{
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}

void Overlay::MoveSelected(const bool up)
{
	if (IsElapsedMax()) return;
	const int modCount = int(mods.size());

	if (up)
	{
		selected--;
		if (selected < 0) selected = modCount - 1;
	}
	else
	{
		selected++;
		if (selected >= modCount) selected = 0;
	}

	refresh = true;
}

bool Overlay::ChangeSelected(const bool minus)
{
	if (IsElapsedMax()) return false;
	Modifier *const mod = mods.at(selected);

	if (minus) mod -> Decrement();
	else mod -> Increment();

	refresh = true;
	return true;
}