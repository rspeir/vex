#include "stdafx.h"
#include "Vex_Win32Rulers.h"
#include "Vex.h"
#include <stdio.h>

#include "cairo-win32.h"
//#include <stdio.h>
//#include <windowsx.h>

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
	//SelectObject(newDC, oldBmp);
	DeleteObject(bmp);
	DeleteObject(oldBmp);
	DeleteDC(newDC);
	EndPaint(hwnd, &paintStruct);
}

LRESULT CALLBACK RulerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	rulerData *data = (rulerData *)GetWindowLongPtr(hWnd, 0);
	RECT parentRect;
	GetClientRect(GetParent(hWnd), &parentRect);
	uint32_t width = parentRect.right - parentRect.left;
	uint32_t height = parentRect.bottom - parentRect.top;
	
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
			if (data->orientation == horizontal)
			{
				MoveWindow(hWnd, RULER_THICKNESS, 0, width, RULER_THICKNESS, TRUE);
			}
			if (data->orientation == vertical)
			{
				MoveWindow(hWnd, 0, RULER_THICKNESS, RULER_THICKNESS, height-46, TRUE);
			}
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
		case WM_SET_RULER_GRIDSUBDIVISIONS:
			data->gridSubdivisions = (uint32_t)wParam;
			break;
		case WM_SET_RULER_GRIDSPACING:
			data->gridSpacing = (uint32_t)wParam;
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

HWND createRuler(HWND parent, HINSTANCE instance, rulerOrientation orientation)
{
	HWND ruler;
	RECT rect;
	GetClientRect(parent, &rect);
	uint32_t width = rect.right - rect.left;
	uint32_t height = rect.bottom - rect.top;
	switch (orientation)
	{
	case vertical:
		ruler = CreateWindowEx(0, TEXT("Ruler"), TEXT("RulerVertical"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, RULER_THICKNESS, RULER_THICKNESS, height, parent, NULL, instance, NULL);
		SendMessage(ruler, WM_SET_RULER_ORIENTATION, vertical, NULL);
		break;
	case horizontal:
		ruler = CreateWindowEx(0, TEXT("Ruler"), TEXT("RulerHorizontal"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, RULER_THICKNESS, 0, width, RULER_THICKNESS, parent, NULL, instance, NULL);
		SendMessage(ruler, WM_SET_RULER_ORIENTATION, horizontal, NULL);
		break;
	case corner:
		ruler = CreateWindowEx(0, TEXT("Ruler"), TEXT("RulerCorner"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, RULER_THICKNESS, RULER_THICKNESS, parent, NULL, instance, NULL);
		SendMessage(ruler, WM_SET_RULER_ORIENTATION, corner, NULL);
		break;
	default:
		ruler = NULL;
		break;
	}
	return ruler;
}

