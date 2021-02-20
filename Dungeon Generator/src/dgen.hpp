#pragma once
#include "vport.hpp"
#include "btree.hpp"

struct Cell;
struct Dungeon;
struct DGManager;

typedef BTree<Cell> Tree;

struct Cell
{
	int xPoint;
	int yPoint;

	SDL_Rect room;
	SDL_Rect space;

	Cell() : xPoint(0), yPoint(0), room{}, space{} {}
};

struct Dungeon
{
	int xSize, ySize;
	DGManager *const mgr;

	Tree tree;

	Dungeon(DGManager *const mgrPtr);
	~Dungeon();

	void Draw();
	void Generate();
	void SetSize(int x, int y) { xSize = x; ySize = y; }
};

struct DGManager
{
	bool quit;

	Dungeon dg;
	Viewport vPort;

	SDL_Window *window;
	SDL_Renderer *renderer;

	DGManager();
	~DGManager();

	void Run();
	void Draw();
	void Update();
};