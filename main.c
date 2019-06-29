#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

#include "game.h"

static const wchar_t *WinClName = L"MyWindowClass";
static HBITMAP bitMap;
static HDC bitHdc = NULL;
static BITMAPINFO bitmapInfo;
static int bitmapSize; 
void *bitmapMem;
uint8_t keys[256];
int screenWidth = 800;
int screenHeight = 640;

HWND WndHandle;

void InitWinBuffer(int width, int height)
{
  if(!bitHdc){
	bitHdc = CreateCompatibleDC(0);
  }
  if(bitmapMem){
	VirtualFree(bitmapMem, 0, MEM_RELEASE);
  }
  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biWidth = width;
  bitmapInfo.bmiHeader.biHeight = -height;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;

  bitmapSize = 4*width*height;
  bitmapMem = VirtualAlloc(NULL, bitmapSize, MEM_COMMIT, PAGE_READWRITE); 
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		case WM_KEYDOWN:
		{
			keys[wParam] = 1;
			break;
		}
		case WM_KEYUP:
		{
			keys[wParam] = 0;
			break;
		}
		case WM_PAINT:
		{
		  PAINTSTRUCT ps;
		  HDC hdc = BeginPaint(hWnd, &ps);
		  StretchDIBits(hdc, 0, 0, bitmapInfo.bmiHeader.biWidth,
			  -bitmapInfo.bmiHeader.biHeight, 0, 0, bitmapInfo.bmiHeader.biWidth,
			  -bitmapInfo.bmiHeader.biHeight, bitmapMem, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
		  EndPaint(hWnd, &ps);
		  break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		default:
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}  
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	WNDCLASSEX wClass = {0};
  	wClass.cbSize = sizeof(WNDCLASSEX);
  	wClass.style = CS_VREDRAW|CS_HREDRAW;
  	wClass.lpfnWndProc = WindowProc;
  	wClass.hInstance = hInstance;
  	wClass.lpszClassName = WinClName;
	wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wClass)){
		MessageBox(NULL, L"RegisterClass", L"Error!", MB_ICONERROR|MB_OK);
		return 0;
	}
	RECT rect = { 0 };
	rect.right = screenWidth;
	rect.bottom = screenHeight;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hWnd = CreateWindowEx(0, WinClName, L"Game v0.1", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
  		CW_USEDEFAULT, rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, hInstance, NULL);

	if(!hWnd){
		MessageBox(NULL, L"CreateWindow", L"Error!", MB_ICONERROR|MB_OK);
		return 0;		
	}
	WndHandle = hWnd;
	GameInit();
	InitWinBuffer(screenWidth, screenHeight);
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);
	MSG msg={0};
	while(1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (GetMessage(&msg, NULL, 0, 0) <= 0) {
				return msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//Game loop
			GameMain(screenWidth, screenHeight);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
		}
	}
	return 1;
}
