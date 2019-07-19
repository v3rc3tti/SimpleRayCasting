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
extern HWND WndHandle;

const char *map = "11234112341111111111111111111111" 
				  "1.........................1....1"
				  "2.........................1....1"
				  "3.........................1....1"
				  "4.........................2....1"
				  "1..............................1"
				  "1..............................1"
			      "1..............................1"
				  "1..............................1"
				  "1..............................1"
				  "1..............................1"
				  "11111111111111111111111111111111";
int mapWidth = 32;
int mapHeight = 12;

float playerX = 2.f;
float playerY = 2.f;
float playerA = M_PI/2.f;
float fov = 60 * (float)M_PI / 180;
float depthBuf[800];

BITMAP bmWall;
Sprite *sprites;

void GameMain(int bufWidth, int bufHeight)
{
	static int oldTime = 0;

	int curTime = GetTickCount();
	int dltTime = curTime - oldTime;
	oldTime = curTime;
	float FPS = (dltTime) ? (1000 / dltTime) : 0;
	wchar_t capt[256];
	swprintf(capt, 256, L"FPS:%f", FPS);
	SetWindowText(WndHandle, capt);

	//Keyboard processing
	ControlProc(dltTime);

	//Rendering
	float step = RAY_STEP;
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
			int hitObj = map[(int)rayY*mapWidth + (int)rayX];
			if (hitObj != '.') {
				hitWall = true;
				depthBuf[x] = distToWall;
				DrawPaling(rayX, rayY, x, ang, distToWall, hitObj - '0');
			}
		}
	}

	DrawSprites(sprites, 1);
	DrawMap(mapWidth, mapHeight);

	int dlt = GetTickCount() - oldTime;
	if (dlt < 33) {
		Sleep(33 - dlt);
	}
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

uint32_t *GetVertLine(BITMAP bm, int lineHeight, int column, int index)
{
	index--;
	int start = 0, end = lineHeight;
	if (lineHeight > screenHeight) {
		start = (lineHeight - screenHeight) / 2;
		end = start + screenHeight;
	}
	uint32_t *line = VirtualAlloc(NULL, screenHeight* sizeof(uint32_t), MEM_COMMIT, PAGE_READWRITE);
	int texHeight = bm.bmHeight;
	int texWidth = bm.bmWidth;
	int section = TEXTURE_WIDTH;
	uint8_t *texture = bm.bmBits;
	int j = 0;
	for (int i = start; i < end; i++) {
		int y = (i * texHeight /(float)lineHeight)+1;
		line[j] |= texture[(texHeight-y)*texWidth*3 + column * 3 + index*section*3]<<16;
		line[j] |= texture[(texHeight-y)*texWidth*3 + column * 3 + index*section*3 + 1]<<8;
		line[j] |= texture[(texHeight-y)*texWidth*3 + column * 3 + index*section*3 + 2];
		j++;
	}
	return line;
}

void FreeLineMem(uint32_t *line)
{
	VirtualFree(line, 0, MEM_RELEASE);
}

void ControlProc(int dTime)
{
	float oldPlayerX = playerX;
	float oldPlayerY = playerY;

	float speedRatio = dTime/500.f;
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
		if (playerA < 0) {
			playerA += 2 * M_PI;
		}
	}
	if (keys[0x45]) {
		playerA += speedRatio;
		if (playerA > 2 * M_PI) {
			playerA -= 2 * M_PI;
		}
	}
	if (map[(int)playerY*mapWidth + (int)playerX] != '.') {
		playerX = oldPlayerX;
		playerY = oldPlayerY;
	}
}

void GameInit(void)
{
	TextureLoader(&bmWall);
}

void DrawPaling(float rayX, float rayY, int x, float ang, float distToWall, int index)
{
	int wallHeight = screenHeight / (distToWall*cosf(ang - playerA));
	if (wallHeight > screenHeight * 5) {
		wallHeight = screenHeight * 5;
	}
	float hitX = rayX - floorf(rayX + 0.5f);
	float hitY = rayY - floorf(rayY + 0.5f);
	float texShift = fabs(hitX) > fabs(hitY) ? rayX : rayY;
	int ceiling = (screenHeight - wallHeight) / 2;
	int floor = wallHeight + ceiling;
	int column = TEXTURE_WIDTH*(texShift-(int)texShift);
	uint32_t *texLine = GetVertLine(bmWall, wallHeight, column, index);
	int l = 0;
	for (int y = 0; y < screenHeight; y++) {
		Color col = { 0 };
		if (y <= ceiling) {
			col.r = 10; col.g = 10; col.b = 10;
		}
		else if (y > ceiling && y < floor) {
			col.r = texLine[l] & 255;
			col.g = (texLine[l] >> 8) & 255;
			col.b = (texLine[l] >> 16) & 255;
			l++;
			if (distToWall > 1.f) {
				col.r /= distToWall;
				col.g /= distToWall;
				col.b /= distToWall;
			}
		}
		else {
			col.r = 10; col.g = 10; col.b = 10;
		}
		DrawPixel(x, y, col);
	}
	FreeLineMem(texLine);
}

void DrawMap(int mapWidth, int mapHeight)
{
	int inch = 6;
	Color col = { 0 };
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			if (map[y*mapWidth + x] == '.') {
				col.r = 0;
				col.g = 100;
				col.b = 0;
			}
			else {
				col.r = 100;
				col.g = 100;
				col.b = 0;
			}
			DrawBox(x, y, inch, col);
		}
	}
	col.r = 0;
	col.g = 0;
	col.b = 255;
	DrawPixel((int)(playerX*inch), (int)(playerY*inch), col);
}

void DrawSprites(Sprite *sprites, int count)
{
	for (int i = 0; i < count; i++) {
		float VecSprX = sprites[i].x - playerX;
		float VecSprY = sprites[i].y - playerY;
		float toSprite = sqrt(VecSprX*VecSprX + VecSprY * VecSprY);
		float angSpr = atan2(VecSprY, VecSprX);
		if (angSpr - playerA > M_PI) angSpr -= 2 * M_PI;
		if (angSpr - playerA < -M_PI) angSpr += 2 * M_PI;
		int width = screenHeight / toSprite;
		width = (width > screenHeight * 2) ? screenHeight * 2 : width;
		float dltAng = angSpr - (playerA - fov / 2);
		int c = (dltAng / fov)*screenWidth;
		int startX = c - width / 2;
		int startY = (screenHeight - width) / 2;

		Color col = { 255.f, 0.f, 0.f };
		uint32_t *screen = bitmapMem;
		for (int y = startY; y < startY + width; y++) {
			if (y < 0 || y >= screenHeight) {continue;}
			for (int x = startX; x < startX + width; x++) {
				if (x < 0 || x >= screenWidth) {continue;}
				DrawPixel(x, y, col);
			}
		}
	}//for sprites
}