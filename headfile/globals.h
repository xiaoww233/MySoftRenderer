#pragma once
#define NOMINMAX
#include <windows.h>

extern HWND renderwindow;
extern const char* g_filePath;
extern int width;
extern int height;
extern TGAImage* screen;
extern Vec3f cameracoord;
extern Vec3f center;
extern Vec3f position;
extern bool bculling;
extern const char* g_filePath;
extern Vec3f light;