#pragma once
#include <wtypes.h>

bool InitializeDirect3d11App(HINSTANCE hInstance, HWND hwnd);
void TerminateDirect3d11App();
bool InitScene(float height, float width);
void UpdateScene();
void DrawScene();
