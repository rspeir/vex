#include "stdafx.h"
#include "Vex_Win32Canvas.h"
#include "Vex.h"
#include <stdio.h>
#include "cairo-win32.h"
#include "Windowsx.h"
#include "Vex_Win32ToolSelector.h"

HWND toolSelector;

HWND createCanvas(HWND parent, HINSTANCE instance)
{
	HWND canvas;
	uint32_t width, height;
	RECT rect;
	GetClientRect(parent, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	canvas = CreateWindow(TEXT("Canvas"), TEXT("Canvas"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_EX_COMPOSITED, RULER_THICKNESS, RULER_THICKNESS, width, height, parent, NULL, instance, NULL);
	return canvas;
}

LRESULT CALLBACK CanvasWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	uint32_t width = rect.right - rect.left;
	uint32_t height = rect.bottom - rect.top;

	RECT parentRect;
	GetClientRect(GetParent(hWnd), &parentRect);
	uint32_t newWidth = parentRect.right - parentRect.left;
	uint32_t newHeight = parentRect.bottom - parentRect.top;

	PAINTSTRUCT paintStruct;
	HDC dc, newDC;
	HGDIOBJ bmp, oldBmp;
	cairo_surface_t *surface;
	cairo_t *cr;

	switch (message)
	{

	case WM_CREATE:
		SetFocus(hWnd);
		registerToolSelectorWindowClass((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
		break;
	
	case WM_SIZE:
		MoveWindow(hWnd, 0, 0, newWidth, newHeight, true);
		break;
	
	case WM_PARENTNOTIFY:
		if (LOWORD(wParam) == WM_DESTROY)
		{
			printf("Child window event detected\n");
			toolSelector = NULL;
		}
		break;

		// ************************* Mouse Stuff *********************************
	case WM_MOUSEMOVE:
		SetFocus(hWnd);
		mouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		mouseDown(Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		mouseUp(Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_MBUTTONDOWN:
		SetCapture(hWnd);

		if ((width > TOOLSELECTOR_SIZE) && (height > TOOLSELECTOR_SIZE))
		{
			if (toolSelector)
			{
				DestroyWindow(toolSelector);
			}
			toolSelector = createToolSelector(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		mouseDown(Middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_MBUTTONUP:
		ReleaseCapture();
		mouseUp(Middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_RBUTTONDOWN:
		SetCapture(hWnd);
		mouseDown(Right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_RBUTTONUP:
		ReleaseCapture();
		mouseUp(Right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_MOUSEWHEEL:
		mouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;

	
	
	case WM_PAINT:
		
		dc = BeginPaint(hWnd, &paintStruct);
		newDC = CreateCompatibleDC(dc);
		bmp = CreateCompatibleBitmap(dc, width, height);
		oldBmp = SelectObject(newDC, bmp);
		surface = cairo_win32_surface_create(newDC);
		cr = cairo_create(surface);

		vexPaintCanvasBackground(cr, width, height);
		vexPaintDrawingObjects(cr, width, height);

		cairo_destroy(cr);
		cairo_surface_destroy(surface);

		BitBlt(dc, 0, 0, width, height, newDC, rect.left, rect.top, SRCCOPY);

		DeleteObject(bmp);
		DeleteObject(oldBmp);
		DeleteDC(newDC);
		EndPaint(hWnd, &paintStruct);
		break;

	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


ATOM registerCanvasWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_PARENTDC;
	wcex.lpfnWndProc = CanvasWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("Canvas");
	wcex.hIconSm = NULL;

	return RegisterClassExW(&wcex);
}
