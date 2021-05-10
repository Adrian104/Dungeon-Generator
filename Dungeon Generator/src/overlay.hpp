#pragma once
#include "global.hpp"
#include "appmgr.hpp"
#include "anim.hpp"

struct Overlay : public Animator
{
	AppManager &mgr;
	SDL_Texture *texture;

	inline int XCenter() const { return (gOverlayWidth - mgr.text.width) >> 1; }

	Overlay(AppManager &appManager);
	~Overlay();

	void Draw();
	void Render();
};