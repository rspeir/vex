#pragma once
#include <Windows.h>
#include "Vex.h"

LRESULT CALLBACK CanvasWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
ATOM registerCanvasWindowClass(HINSTANCE hInstance);
HWND createCanvas(HWND parent, HINSTANCE instance);
void resizeCanvas(UINT message, WPARAM wParam, LPARAM lParam);
