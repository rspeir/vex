#pragma once
#include <Windows.h>
#include "Vex.h"

#define WM_SET_RULER_ORIENTATION 0x8002
#define WM_SET_RULER_UNITS 0x8003
#define WM_SET_RULER_GRIDSPACING 0x8004
#define WM_SET_RULER_GRIDSUBDIVISIONS 0x8005

#define RULER_THICKNESS 28

LRESULT CALLBACK RulerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
ATOM registerRulerWindowClass(HINSTANCE hInstance);

typedef struct
{
	unitType units;
	uint32_t gridSpacing;
	uint32_t gridSubdivisions;
	int32_t pan;
	rulerOrientation orientation;
} rulerData;
