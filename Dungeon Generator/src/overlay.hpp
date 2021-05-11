#pragma once
#include <vector>
#include "global.hpp"
#include "appmgr.hpp"
#include "anim.hpp"

struct RefInterface
{
	std::string name;

	RefInterface(const std::string &pName) : name(pName) {}
	virtual std::string Get() = 0;

	virtual void Plus() = 0;
	virtual void Minus() = 0;
};

struct Overlay : public Animator
{
	int selected;
	bool refresh;

	AppManager &mgr;
	SDL_Texture *texture;
	std::vector<RefInterface*> refs;

	inline int XCenter() const { return (gOverlayWidth - mgr.text.width) >> 1; }

	Overlay(AppManager &appManager);
	~Overlay();

	void Draw();
	void Render();
	bool Update();

	void MoveSelected(const bool up);
	bool ChangeSelected(const bool minus);

	template <typename Type>
	inline void AddRef(const Type &ref) { refs.push_back(new Type(ref)); }
};

template <typename Type>
struct ValRef : public RefInterface
{
	Type *ref;

	ValRef(const std::string &pName, Type *pRef) : RefInterface(pName), ref(pRef) {}
	std::string Get() override { return std::to_string(*ref); }

	void Plus() override { ++(*ref); }
	void Minus() override { --(*ref); if (*ref < 0) *ref = 0; }
};

template <typename Type>
struct PercRef : public RefInterface
{
	Type *ref;

	PercRef(const std::string &pName, Type *pRef) : RefInterface(pName), ref(pRef) {}
	std::string Get() override { return std::to_string(*ref) + " %"; }

	void Plus() override { *ref += 5; if (*ref > 100) *ref = 100; }
	void Minus() override { *ref -= 5; if (*ref < 0) *ref = 0; }
};

struct BoolRef : public RefInterface
{
	bool *ref;

	BoolRef(const std::string &pName, bool *pRef) : RefInterface(pName), ref(pRef) {}
	std::string Get() override { return *ref ? "Enabled" : "Disabled"; }

	void Plus() override { *ref = !(*ref); }
	void Minus() override { *ref = !(*ref); }
};

struct FactRef : public RefInterface
{
	float *ref;

	FactRef(const std::string &pName, float *pRef) : RefInterface(pName), ref(pRef) {}
	std::string Get() override { return std::to_string(*ref); }

	void Plus() override { *ref *= 1.1f; }
	void Minus() override { *ref *= 0.9f; }
};

template <size_t size>
struct ModeRef : public RefInterface
{
	int *ref;
	const std::string *modeNames;

	ModeRef(const std::string &pName, int *pRef, const std::string *pModeNames) : RefInterface(pName), ref(pRef), modeNames(pModeNames) {}
	std::string Get() override { return modeNames[*ref]; }

	void Plus() override { ++(*ref); if (*ref >= size) *ref = 0; }
	void Minus() override { --(*ref); if (*ref < 0) *ref = (size - 1); }
};