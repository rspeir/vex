#include "stdafx.h"
#include "Vex.h"
#include <stdio.h>

#include "cairo-win32.h"
#include "Vex_Win32ToolSelector.h"

//Tools:
//typedef enum { nulltool, pentool, pointselect, groupselect, bezier } toolType;

typedef enum
{
#define ENUM_MAP() \
	ENUM_MEMBER(MEMBER1, 0) \
	ENUM_MEMBER(MEMBER2, 10) \
	ENUM_MEMBER(MEMBER3, 20)
#define ENUM_MEMBER(n,v) n=v, 
	ENUM_MAP()
#undef ENUM_MEMBER
#define ENUM_MEMBER(n,v) + 1
	COUNT = ENUM_MAP()
#undef ENUM_MEMBER
#undef ENUM_MAP
}enums;

HWND createToolSelector(HWND parent, HINSTANCE instance, int32_t x, int32_t y)
{
	HWND toolSelector;

	RECT rect;
	GetClientRect(parent, &rect);

	int32_t centerX = x - TOOLSELECTOR_SIZE / 2;
	int32_t centerY = y - TOOLSELECTOR_SIZE / 2;


	toolSelector = CreateWindow(TEXT("ToolSelector"), TEXT("ToolSelector"), WS_CHILD | WS_VISIBLE, centerX, centerY, TOOLSELECTOR_SIZE, TOOLSELECTOR_SIZE, parent, NULL, instance, NULL);
	printf("tool selector pointer %p\n", toolSelector);

	return toolSelector;
}

LRESULT CALLBACK ToolSelectorWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		printf("tool selector being created %d\n", sizeof(toolType));
		
		break;
	case WM_PAINT:
		break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONUP:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		printf("Destroying tool selector\n");
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

ATOM registerToolSelectorWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_PARENTDC;
	wcex.lpfnWndProc = ToolSelectorWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(int);
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("ToolSelector");
	wcex.hIconSm = NULL;

	return RegisterClassExW(&wcex);

}

