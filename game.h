#ifndef GAME_H
#define GAME_H

#define RAY_STEP 0.01f

typedef struct {
	int r;
	int g;
	int b;
}Color;
void GameInit(void);
void GameMain(int screenWidth, int screenHeight);
void DrawPixel(int x, int y, Color col);
void DrawBox(int left, int top, int inch, Color col);
void DrawMap(int mapWidth, int mapHeight);
void TextureLoader(BITMAP *bm);
uint32_t *GetVertLine(BITMAP bm, int lineHeight, int column);
void FreeLineMem(uint32_t *line);
void DrawPaling(float rayX, float rayY, int x, float ang, float distToWall);
void ControlProc(int dTime);
int	 CalcDeltaTime(void);

#endif
