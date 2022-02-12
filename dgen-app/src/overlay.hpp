#pragma once
#include <cmath>
#include <string>
#include <vector>
#include <limits>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "appmgr.hpp"
#include "anim.hpp"
#include "global.hpp"

struct Modifier
{
	std::string name;

	Modifier(const std::string &pName) : name(pName) {}
	virtual ~Modifier() {}

	virtual void Increment() = 0;
	virtual void Decrement() = 0;

	virtual std::string GetValue() const = 0;
};

struct Overlay : public Animator
{
	int selected;
	bool refresh;

	AppManager &mgr;
	SDL_Texture *texture;
	std::vector<Modifier*> mods;

	int XCenter(Text& text) const;

	Overlay(AppManager &appManager);
	~Overlay();

	void Draw();
	void Render();
	bool Update();

	void InitResources();
	void DestroyResources();

	void MoveSelected(const bool up);
	bool ChangeSelected(const bool minus);

	template <typename Type>
	inline void AddMod(const Type &mod) { mods.push_back(new Type(mod)); }
};

struct FactorMod : public Modifier
{
	float &ref;
	float step;

	FactorMod(const std::string &pName, float &pRef, float pStep = 0.1f) : Modifier(pName), ref(pRef), step(pStep) {}

	void Increment() override { ref *= (1.0f + step); }
	void Decrement() override { ref *= (1.0f - step); }

	std::string GetValue() const override { return std::to_string(ref); }
};

struct PercentMod : public Modifier
{
	float &ref;
	float step;

	PercentMod(const std::string &pName, float &pRef, float pStep = 0.05f) : Modifier(pName), ref(pRef), step(pStep) {}

	void Increment() override { ref += step; if (ref > 1.0f) ref = 1.0f; }
	void Decrement() override { ref -= step; if (ref < 0.0f) ref = 0.0f; }

	std::string GetValue() const override { return std::to_string(int(std::round(ref * 100.0f))) + " %"; }
};

struct BoolMod : public Modifier
{
	bool &ref;

	BoolMod(const std::string &pName, bool &pRef) : Modifier(pName), ref(pRef) {}

	void Increment() override { ref = !ref; }
	void Decrement() override { ref = !ref; }

	std::string GetValue() const override { return ref ? "Enabled" : "Disabled"; }
};

struct IntMod : public Modifier
{
	int &ref;
	int min, max;

	IntMod(const std::string &pName, int &pRef, int pMin = 0, int pMax = std::numeric_limits<int>::max()) : Modifier(pName), ref(pRef), min(pMin), max(pMax) {}

	void Increment() override { if (++ref > max) ref = max; }
	void Decrement() override { if (--ref < min) ref = min; }

	std::string GetValue() const override { return std::to_string(ref); }
};