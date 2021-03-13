#pragma once
#include <cstdlib>
#include <time.h>
#include "vport.hpp"
#include "btree.hpp"

struct Cell;
struct Dungeon;
struct DGManager;

typedef BTree<Cell> Tree;

struct Room
{
	Room *nextRoom;
	SDL_FRect room;

	Room() : nextRoom(nullptr), room{} {}
	~Room() { delete nextRoom; }
};

struct Cell
{
	Room *roomList;
	SDL_FRect space;

	Cell() : roomList(nullptr), space{} {}
	~Cell() { delete roomList; }
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