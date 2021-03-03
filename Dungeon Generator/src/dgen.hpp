#pragma once
#include <cstdlib>
#include <time.h>
#include <map>
#include "vport.hpp"
#include "btree.hpp"

struct Cell;
struct Dungeon;
struct DGManager;

typedef BTree<Cell> Tree;

struct Cell
{
	SDL_Rect space;
	SDL_Point point;

	Cell() : space{}, point{} {}
};

struct Path
{
	SDL_Point xLines[3];
	SDL_Point yLines[3];
};

struct GenInfo
{
	int xSize, ySize;
	int maxDepth, minDepth;
	int spaceSizeRandomness;
	int maxRoomSize, minRoomSize;
};

struct DrawInfo
{
	bool spaceVisibility;
	bool roomsVisibility;
	bool pointsVisibility;
};

struct Dungeon
{
	Tree tree;
	std::multimap<Cell*, SDL_Rect> rooms;

	DGManager &mgr;
	GenInfo &gInfo;
	DrawInfo &dInfo;

	void GeneratePaths();
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