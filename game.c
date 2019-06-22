#define _USE_MATH_DEFINES
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <windows.h>

#include "game.h"

extern void *bitmapMem; //screen memory 
extern int screenWidth;
extern int screenHeight;
extern uint8_t keys[256];

const char *map = "xxxxxxxxxxxxxxxx" 
				  "x...........x..x"
				  "x...........x..x"
				  "x...........x..x"
				  "x...........x..x"
				  "x.....xxxxxxx..x"
				  "x..............x"
			      "x..............x"
				  "x..............x"
				  "xxxxxxxxxxxxxxxx";
int mapWidth = 16;
int mapHeight = 10;

float playerX = 2.f;
float playerY = 2.f;
float playerA = 0.f;
float fov = 60 * (float)M_PI / 180;
int oldTime, curTime;

void GameMain(int bufWidth, int bufHeight)
{
	//Keys processing
	float oldPlayerX = playerX;
	float oldPlayerY = playerY;

	float speedRatio = 0.1f;
	if (keys[0x57]) {
		playerX += cosf(playerA)*speedRatio;
		playerY += sinf(playerA)*speedRatio;
	}
	if (keys[0x53]) {
		playerX -= cosf(playerA)*speedRatio;
		playerY -= sinf(playerA)*speedRatio;
	}
	if (keys[0x41]) {
		playerX += cosf(playerA - 0.5f*M_PI)*speedRatio;
		playerY += sinf(playerA - 0.5f*M_PI)*speedRatio;
	}
	if (keys[0x44]) {
		playerX += cosf(playerA + 0.5f*M_PI)*speedRatio;
		playerY += sinf(playerA + 0.5f*M_PI)*speedRatio;
	}
	if (keys[0x51]) {
		playerA -= speedRatio;
	}
	if (keys[0x45]) {
		playerA += speedRatio;
	}
	if (map[(int)playerY*mapWidth + (int)playerX] == 'x') {
		playerX = oldPlayerX;
		playerY = oldPlayerY;
	}

	//Rendering
	float step = 0.01f;
	float ang = playerA - fov / 2;
	float dAng = 1.f / bufWidth;
	for (int x = 0; x < bufWidth; x++) {
		ang += dAng;
		float dx = cosf(ang)*step;
		float dy = sinf(ang)*step;
		bool hitWall = false;
		float rayX = playerX;
		float rayY = playerY;
		float distToWall = 0.f;
		while (!hitWall) {
			rayX += dx;
			rayY += dy;
			distToWall += step;
			if (map[(int)rayY*mapWidth + (int)rayX] == 'x') {
				hitWall = true;
				int wallHeight = screenHeight / (distToWall*cosf(ang-playerA));
				int ceiling = (screenHeight-wallHeight) / 2;
				int floor = wallHeight + ceiling;
				for (int y = 0; y < screenHeight; y++) {
					Color col = { 0 };
					if (y < ceiling) {
						col.r = 0; col.g = 0; col.b = 0;
					}
					else if (y >= ceiling && y <= floor) {
						col.r = 255/distToWall; col.g = 255/distToWall; col.b = 255/distToWall;
					}
					else {
						col.r = 0; col.g = 0; col.b = 0;
					}
					DrawPixel(x, y, col);
				}
			}
		}
	}
	DrawMap(mapWidth, mapHeight);
	Sleep(30);
}

void DrawMap(int mapWidth, int mapHeight)
{
	int inch = 6;
	Color col = { 0 };
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			switch (map[y*mapWidth + x]) {
			case 'x':
				col.r = 255;
				col.g = 255;
				col.b = 0;
				break;
			case '.':
				col.r = 255;
				col.g = 0;
				col.b = 0;
				break;
			}
			DrawBox(x, y, inch, col);
		}
	}
	col.r = 0;
	col.g = 0;
	col.b = 255;
	DrawPixel((int)(playerX*inch), (int)(playerY*inch), col);
}

void DrawBox(int left, int top, int inch, Color col)
{
	int x1 = left * inch;
	int y1 = top * inch;
	int x2 = x1 + inch;
	int y2 = y1 + inch;
	for (int x =  x1; x < x2; x++) {
		for (int y = y1; y < y2; y++) {
			DrawPixel(x, y, col);
		}
	}
}

void DrawPixel(int x, int y, Color col)
{
	uint32_t *screen = bitmapMem;
	screen[y*screenWidth + x] = (col.r << 16) | (col.g << 8) | col.b;
}
