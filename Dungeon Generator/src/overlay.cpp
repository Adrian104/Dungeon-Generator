#include "pch.hpp"
#include "overlay.hpp"

const int gOverlayXMargin2 = gOverlayXMargin << 1;
const int gOverlayYMargin2 = gOverlayYMargin << 1;

Overlay::Overlay(AppManager &appManager) : Animator(std::chrono::milliseconds(gOverlayAnimTime)), selected(0), refresh(true), mgr(appManager)
{
	texture = SDL_CreateTexture(mgr.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gOverlayWidth, mgr.windowHeight);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
}

Overlay::~Overlay()
{
	for (RefInterface *&ref : refs) delete ref;
	SDL_DestroyTexture(texture);
}

void Overlay::Draw()
{
	const SDL_Rect dest = { int(-gOverlayWidth * GetAnimPhase()), 0, gOverlayWidth, mgr.windowHeight };
	SDL_RenderCopy(mgr.renderer, texture, nullptr, &dest);
}

void Overlay::Render()
{
	SDL_Rect rect = { gOverlayWidth - gOverlayOutlineWidth, 0, gOverlayOutlineWidth, mgr.windowHeight };

	SDL_SetRenderTarget(mgr.renderer, texture);
	SDL_SetRenderDrawColor(mgr.renderer, 0, 0, 0, 0xCC);
	SDL_RenderClear(mgr.renderer);

	SDL_SetRenderDrawColor(mgr.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(mgr.renderer, &rect);

	AppManager::TextTx &text = mgr.text;
	mgr.RenderText(gTitle, 0, TTF_STYLE_BOLD);

	int xCenter = XCenter();

	rect = { xCenter, gOverlayTitleYPos, text.width, text.height };
	SDL_RenderCopy(mgr.renderer, text.texture, nullptr, &rect);

	rect = { xCenter - gOverlayXMargin, gOverlayTitleYPos - gOverlayYMargin, text.width + gOverlayXMargin2, text.height + gOverlayYMargin2 };
	SDL_RenderDrawRect(mgr.renderer, &rect);

	int crrIndex = 0;
	int crrPos = gOverlayTitleYPos + gOverlayTitleYOffset;

	for (RefInterface *&ref : refs)
	{
		const uint8_t colorVal = crrIndex == selected ? 0 : 0xFF;

		mgr.RenderText(ref -> name, 0, TTF_STYLE_BOLD, { colorVal, 0xFF, colorVal, 0xFF });
		xCenter = XCenter();

		rect = { xCenter, crrPos, text.width, text.height };
		SDL_RenderCopy(mgr.renderer, text.texture, nullptr, &rect);
		crrPos += gOverlayInternalOffset;

		mgr.RenderText(ref -> Get(), 0, TTF_STYLE_NORMAL);
		xCenter = XCenter();

		rect = { xCenter, crrPos, text.width, text.height };
		SDL_RenderCopy(mgr.renderer, text.texture, nullptr, &rect);
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
		if (IsAnimating()) UpdateAnim();

		return true;
	}

	if (IsAnimating())
	{
		UpdateAnim();
		return true;
	}

	return false;
}

void Overlay::MoveSelected(const bool up)
{
	if (GetAnimPhase() == 1.0f) return;
	const int refCount = int(refs.size());

	if (up)
	{
		selected--;
		if (selected < 0) selected = refCount - 1;
	}
	else
	{
		selected++;
		if (selected >= refCount) selected = 0;
	}

	refresh = true;
}

bool Overlay::ChangeSelected(const bool minus)
{
	if (GetAnimPhase() == 1.0f) return false;
	RefInterface *const ref = refs.at(selected);

	if (minus) ref -> Minus();
	else ref -> Plus();

	refresh = true;
	return true;
}