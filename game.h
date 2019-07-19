#ifndef GAME_H
#define GAME_H

#define RAY_STEP 0.01f
#define TEXTURE_WIDTH 100

typedef struct {
	int r;
	int g;
	int b;
}Color;

typedef struct {
	float x;
	float y; 
	int index;
}Sprite;

void GameInit(void);
void GameMain(int screenWidth, int screenHeight);
void DrawPixel(int x, int y, Color col);
void DrawBox(int left, int top, int inch, Color col);
void DrawMap(int mapWidth, int mapHeight);
void TextureLoader(BITMAP *bm);
uint32_t *GetVertLine(BITMAP bm, int lineHeight, int column, int index);
void FreeLineMem(uint32_t *line);
void DrawPaling(float rayX, float rayY, int x, float ang, float distToWall, int index);
void ControlProc(int dTime);
void DrawSprites(Sprite sprites, int count);

#endif
