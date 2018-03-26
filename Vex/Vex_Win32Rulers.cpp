#include "stdafx.h"
#include "Vex_Win32Rulers.h"
#include "Vex.h"
#include <stdio.h>

#include "cairo-win32.h"
//#include <stdio.h>
//#include <windowsx.h>



static void sizeRuler(HWND hwnd, rulerData *data, uint32_t width, uint32_t height)
{
	switch (data->orientation)
	{
	case horizontal:
		SetWindowPos(hwnd, HWND_TOP, RULER_THICKNESS, 0, width, RULER_THICKNESS, SWP_SHOWWINDOW);
		break;
	case vertical:
		SetWindowPos(hwnd, HWND_TOP, 0, RULER_THICKNESS, RULER_THICKNESS, height, SWP_SHOWWINDOW);
	default:
		break;
	}
}

static void paintRuler(HWND hwnd, rulerData *data)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	uint32_t width = rect.right - rect.left;
	uint32_t height = rect.bottom - rect.top;

	PAINTSTRUCT paintStruct;
	HDC dc, newDC;

	HGDIOBJ bmp, oldBmp;
	cairo_surface_t *surface;
	cairo_t *cr;

	dc = BeginPaint(hwnd, &paintStruct);
	newDC = CreateCompatibleDC(dc);
	bmp = CreateCompatibleBitmap(dc, width, height);
	oldBmp = SelectObject(newDC, bmp);
	surface = cairo_win32_surface_create(newDC);
	cr = cairo_create(surface);

	vexPaintRuler(cr, width, height, data->orientation);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	BitBlt(dc, 0, 0, width, height, newDC, rect.left, rect.top, SRCCOPY);
	SelectObject(newDC, oldBmp);
	DeleteDC(newDC);
	EndPaint(hwnd, &paintStruct);
}

LRESULT CALLBACK RulerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	rulerData *data = (rulerData *)GetWindowLongPtr(hWnd, 0);
	RECT parentRect;
	GetClientRect(GetParent(hWnd), &parentRect);
	
	switch (message)
	{
		case WM_NCCREATE:
			data = (rulerData *)malloc(sizeof(rulerData));
			SetWindowLongPtr(hWnd, 0, (LONG_PTR)data);
			data->units = vexGetProgramState()->currUnitType;
			data->gridSpacing = vexGetProgramState()->gridSpacing;
			data->gridSubdivisions = vexGetProgramState()->gridSubdivisions;
			data->orientation = undefined;
			break;
		case WM_NCDESTROY:
			if (data != NULL)
				free(data);
			break;
		case WM_SIZE:
			sizeRuler(hWnd, data, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_PAINT:
			paintRuler(hWnd, data);
			break;
		case WM_SET_RULER_ORIENTATION:
			data->orientation = (rulerOrientation)wParam;
			break;
		case WM_SET_RULER_UNITS:
			data->units = (unitType)wParam;
			break;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


ATOM registerRulerWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_PARENTDC;
	wcex.lpfnWndProc = RulerWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(rulerData);
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("Ruler");
	wcex.hIconSm = NULL;

	return RegisterClassExW(&wcex);
}

