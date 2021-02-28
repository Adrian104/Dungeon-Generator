#pragma once
#include <cstdlib>
#include <time.h>
#include "vport.hpp"
#include "btree.hpp"

struct Cell;
struct Dungeon;
struct DGManager;

typedef BTree<Cell> Tree;

struct Cell
{
	SDL_Rect room;
	SDL_Rect space;
	SDL_Point point;

	Cell() : room{}, space{}, point{} {}
};

struct GenInfo
{
	int xSize, ySize;
	int maxDepth, minDepth;
	int spaceSizeRandomness;
};

struct DrawInfo
{
	bool spaceVisibility;
};

struct Dungeon
{
	Tree tree;
	DGManager &mgr;
	GenInfo &gInfo;
	DrawInfo &dInfo;

	void Divide(int left);

	Dungeon(DGManager &pMgr, GenInfo &pGInfo, DrawInfo &pDInfo);
	~Dungeon();

	void Draw();
	void Generate();
};

struct DGManager
{
	bool quit;
	bool needRedraw;

	Dungeon dg;
	GenInfo gInfo;
	DrawInfo dInfo;
	Viewport vPort;

	SDL_Window *window;
	SDL_Renderer *renderer;

	DGManager();
	~DGManager();

	void Run();
	void Draw();
	void Update();
};