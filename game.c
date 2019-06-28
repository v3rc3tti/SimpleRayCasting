#define _USE_MATH_DEFINES
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <windows.h>
#include <stdio.h>

#include "game.h"

extern void *bitmapMem; //screen memory 
extern int screenWidth;
extern int screenHeight;
extern uint8_t keys[256];
extern BITMAP bmWall;

const char *map = "xxxxxxxxxxxxxxxx" 
				  "x...........x..x"
				  "x...........x..x"
				  "x...........x..x"
				  "x...........x..x"
				  "x.....xxxxxxx..x"
				  "x..............x"
			      "x..............x"
				  "x..............x"
				  "x..............x"
				  "x..............x"
				  "xxxxxxxxxxxxxxxx";
int mapWidth = 16;
int mapHeight = 12;

float playerX = 2.f;
float playerY = 2.f;
float playerA = 0.5f;
float fov = 60 * (float)M_PI / 180;
int oldTime, curTime;

extern whd;
void GameMain(int bufWidth, int bufHeight)
{
	curTime = GetTickCount();
	int dltTime = curTime - oldTime;
	oldTime = curTime;
	float FPS = 1000 / dltTime;
	wchar_t capt[256];
	swprintf(capt, 256, L"FPS:%f", FPS);
	SetWindowText(whd, capt);
	//Keys processing
	KeyboardProc();

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
				if (wallHeight > screenHeight*5) {
					wallHeight = screenHeight *5;
				}
				int ceiling = (screenHeight-wallHeight) / 2;
				int floor = wallHeight + ceiling;
				float hitX = rayX - floorf(rayX+0.5f);
				float hitY = rayY - floorf(rayY+0.5f);
				float hitnom = fabs(hitX) > fabs(hitY) ? hitX : hitY;
				int column = bmWall.bmWidth*(hitnom)+bmWall.bmWidth;
				uint32_t *texLine = GetVertLine(bmWall, wallHeight, column);
				int l = 0;
				for (int y = 0; y < screenHeight; y++) {
					Color col = { 0 };
					if (y < ceiling) {
						col.r = 0; col.g = 0; col.b = 0;
					}
					else if (y >= ceiling && y <= floor) {
						col.r = texLine[l] & 255;
						col.g = (texLine[l] >> 8) & 255;
						col.b = (texLine[l] >> 16) & 255;
						l++;
						if (distToWall > 1.f) {
							col.r /= distToWall;
							col.g /= distToWall;
							col.b /= distToWall;
						}
						/*else if(distToWall<0.5f){
							col.r *= distToWall;
							col.g *= distToWall;
							col.b *= distToWall;
						}*/
					}
					else {
						col.r = 0; col.g = 0; col.b = 0;
					}
					DrawPixel(x, y, col);
				}
				FreeLineMem(texLine);
			}
		}
	}
	DrawMap(mapWidth, mapHeight);
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

void TextureLoader(BITMAP *bm)
{
	HBITMAP hTexture = LoadImage(NULL, L"texture.bmp", IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	GetObject(hTexture, sizeof(BITMAP), bm);
}

uint32_t *GetVertLine(BITMAP bm, int lineHeight, int column)
{
	int start = 0, end = lineHeight;
	if (lineHeight > screenHeight) {
		start = (lineHeight - screenHeight) / 2;
		end = (start + screenHeight);
	}
	uint32_t *line = VirtualAlloc(NULL, screenHeight * sizeof(uint32_t), MEM_COMMIT, PAGE_READWRITE);
	int texHeight = bm.bmHeight;
	int texWidth = bm.bmWidth;
	uint8_t *texture = bm.bmBits;
	int j = 0;
	for (int i = start; i < end; i++) {
		int y = (i * texHeight) /lineHeight;
		line[j] |= texture[y*texWidth*3 + column*3]<<16;
		line[j] |= texture[y*texWidth*3 + column * 3+1]<<8;
		line[j] |= texture[y*texWidth*3 + column * 3+2];
		j++;
	}
	return line;
}

void FreeLineMem(uint32_t *line)
{
	VirtualFree(line, 0, MEM_RELEASE);
}

void KeyboardProc(void)
{
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
}