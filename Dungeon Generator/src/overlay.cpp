#include "overlay.hpp"

Overlay::Overlay(AppManager &appManager) : Animator(std::chrono::milliseconds(gOverlayAnimTime)), mgr(appManager)
{
	texture = SDL_CreateTexture(mgr.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gOverlayWidth, mgr.windowHeight);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
}

Overlay::~Overlay() { SDL_DestroyTexture(texture); }

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
	mgr.RenderText("Dungeon Generator", 0, TTF_STYLE_BOLD);

	int xCenter = XCenter();

	rect = { xCenter, gOverlayTitleYPos, text.width, text.height };
	SDL_RenderCopy(mgr.renderer, text.texture, nullptr, &rect);

	rect = { xCenter - gOverlayXMargin, gOverlayTitleYPos - gOverlayYMargin, text.width + gOverlayXMargin2, text.height + gOverlayYMargin2 };
	SDL_RenderDrawRect(mgr.renderer, &rect);
}