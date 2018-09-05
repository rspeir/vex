#pragma once
#include <Windows.h>
#include "Vex.h"

#define TOOLSELECTOR_SIZE 200

LRESULT CALLBACK ToolSelectorWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
ATOM registerToolSelectorWindowClass(HINSTANCE hInstance);
HWND createToolSelector(HWND parent, HINSTANCE instance, int32_t x, int32_t y);