#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "raylib.h"
#include "raymath.h"

typedef struct Cell
{
	bool isAlive;
} Cell;

static Cell *cells;
const size_t cellsCount = 250*250;
const float zoomSpeed = 50;
const float zoomSens = 0.1f;
static const size_t cellSize = 40;
static bool isPause = false;

extern void initCells(size_t count);
extern void drawField(void);
extern void drawCells(void);
extern void updateCells(float dt);
extern void getAdjacent(size_t i, Cell **out);

int main(void)
{
	InitWindow(600, 600, "Game Of Life");
	initCells(cellsCount);
	Camera2D cam = {0};
	cam.target = (Vector2){20.f, 20.f};
	cam.rotation = 0.f;
	cam.zoom = 1.f;
	SetTargetFPS(60);
	while(!WindowShouldClose())
	{
		float deltaTime = GetFrameTime();
		Vector2 mPos = GetScreenToWorld2D(GetMousePosition(), cam);
		if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			int clampX = (mPos.x / cellSize) * cellSize + cellSize/2;
			int clampY = (mPos.y / cellSize) * cellSize + cellSize/2;
			int xc = clampX/cellSize;
			int yc = clampY/cellSize;
			if(xc >= 0 && xc <= sqrt(cellsCount)
				&& yc >= 0 && yc <= sqrt(cellsCount))
			{
				size_t i = sqrt(cellsCount) * yc + xc;
				cells[i].isAlive = true;
			}
		}
		if(IsKeyPressed(KEY_C))
			memset(cells, 0, sizeof(Cell) * cellsCount);
		if(IsKeyPressed(KEY_P))
			isPause = !isPause;
		if(IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
		{
			Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f/cam.zoom);
            cam.target = Vector2Add(cam.target, delta);
		}
		if(GetMouseWheelMove() != 0)
		{
			cam.offset = GetMousePosition();
			cam.target = mPos;
			cam.zoom = Lerp(cam.zoom, Clamp(cam.zoom + GetMouseWheelMove() * zoomSens, 0.13f, 5), zoomSpeed * deltaTime);
		}
		if(!isPause)
		{
			updateCells(deltaTime);
		}

		BeginDrawing();
		{
			ClearBackground(WHITE);
			BeginMode2D(cam);
			{
				drawField();
				drawCells();
			}
			EndMode2D();
		}
		EndDrawing();

	}
	free(cells);
	CloseWindow();
	return 0;
}

void initCells(size_t count)
{
	cells = (Cell *) malloc(sizeof(Cell) * count);
	for(size_t i = 0; i < cellsCount; ++i)
	{
		cells[i].isAlive = false;
	}
}

void drawField(void)
{
	for(size_t x = 0; x < sqrt(cellsCount) * cellSize; x += cellSize)
	{
		DrawLine(x, 0, x, sqrt(cellsCount) * cellSize, BLACK);
	}
	for(size_t y = 0; y < sqrt(cellsCount) * cellSize; y += cellSize)
	{
		DrawLine(0, y, sqrt(cellsCount) * cellSize, y, BLACK);
	}
}

void drawCells(void)
{
	for(size_t i = 0; i < cellsCount; ++i)
	{
		if(cells[i].isAlive)
		{
			int y = i/sqrt(cellsCount);
			int x = i - y * sqrt(cellsCount);
			DrawCircle(x * cellSize + cellSize/2, 
						y * cellSize + cellSize/2, 
						cellSize/2, GREEN);
		}
	}
}

void getAdjacent(size_t i, Cell **out)
{	
	unsigned short count = 0;

	int sq = (int)sqrt(cellsCount);

	bool hasRight = i%sq < sq;
	bool hasLeft = i%sq > 1;
	bool hasUpper = i > sq;
	bool hasDown = i < cellsCount - sq;

	out[0] = hasRight ? &cells[i + 1] : NULL;
	out[1] = hasLeft ? &cells[i - 1] : NULL;	
	out[3] = hasUpper ? &cells[i - sq] : NULL;
	out[2] = hasDown ? &cells[i + sq] : NULL;
	out[4] = hasRight && hasUpper ? &cells[i - sq + 1] : NULL;
	out[5] = hasLeft && hasUpper ? &cells[i - sq - 1] : NULL;
	out[6] = hasRight && hasDown ? &cells[i + sq + 1] : NULL;
	out[7] = hasDown && hasLeft ? &cells[i + sq - 1] : NULL;
}

static float tt;
void updateCells(float dt)
{
	if(tt < 1.f/20)
	{
		tt += dt;
		return;
	}
	tt = 0;

	Cell nextCells[cellsCount];
	memset(nextCells, 0, cellsCount * sizeof(Cell));

	for(size_t i = 0; i < cellsCount; ++i)
	{
		Cell *adj[8] = {0}; 
		getAdjacent(i, adj);
		
		uint8_t alive = 0;
		for(uint8_t y = 0; y < 8; ++y)
		{
			if(adj[y])
			{
				alive += adj[y]->isAlive;
			}
		}


		if(cells[i].isAlive)
		{
			if(alive < 2 || alive > 3)
			{
				nextCells[i].isAlive = false;
			}
			else
			{
				nextCells[i].isAlive = true;
			}
		}
		else if(alive == 3)
		{
			nextCells[i].isAlive = true;
		}

	}
	memcpy(cells, nextCells, cellsCount * sizeof(Cell));
}