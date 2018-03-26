// Vex.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <Strsafe.h>
#include <cairo-win32.h>

#include "Vex.h"
#include "Vex_Win32UnitsGrid.h"
#include "Vex_Win32Rulers.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND statusBar;
HWND rulerHorizontal;
HWND rulerVertical;
HWND rulerCorner;

void getDisplayInfo()
{
	DISPLAY_DEVICE dd = {};
	dd.cb = sizeof(dd);
	int counter = 0;
	int  numMonitors = 0;
	while (EnumDisplayDevices(NULL, counter, &dd, NULL))
	{
		wprintf(dd.DeviceName);
		printf(" ");
		wprintf(dd.DeviceString);
		printf(" ");
		if (dd.StateFlags && DISPLAY_DEVICE_ACTIVE)
		{
			printf("<-- Active");
			numMonitors++;
		}
		printf("\n");
		counter++;
	}

	HDC screen = GetDC(NULL);
	int xdpi = GetDeviceCaps(screen, LOGPIXELSX);
	int ydpi = GetDeviceCaps(screen, LOGPIXELSY);
	int xsize = GetDeviceCaps(screen, HORZSIZE);
	int ysize = GetDeviceCaps(screen, VERTSIZE);
	int xres = GetDeviceCaps(screen, HORZRES);
	int yres = GetDeviceCaps(screen, VERTRES);
	ReleaseDC(NULL, screen);

	printf("Found %d active monitor(s).\n", numMonitors);
	printf("X pixels per inch: %d  Y pixels per inch: %d.\n", xdpi, ydpi);
	printf("Screen is %d mm wide by %d mm high.\n", xsize, ysize);
	printf("That translates to %f by %f inches.\n", convertUnits(xsize, millimeters, inches), convertUnits(ysize, millimeters, inches));
	printf("Screen is %d pixels wide by %d pixels high.\n", xres, yres);

	vexSetProgramStateDisplayInfo(xdpi);

}


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    UnitsGrid(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VEX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
	registerRulerWindowClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VEX));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

#ifdef _DEBUG
int main()
{
	return wWinMain(GetModuleHandle(0), NULL, NULL, SW_SHOW);
}
#endif


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class for the main application window.
//
static ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VEX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VEX);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


void resizeRulers(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (rulerHorizontal)
	{
		SendMessage(rulerHorizontal, message, wParam, lParam);
	}
	if (rulerVertical)
	{
		SendMessage(rulerVertical, message, wParam, lParam);
	}
	if (rulerCorner)
	{
		SendMessage(rulerCorner, message, wParam, lParam);
	}
}

static HWND createRuler(HWND parent, HINSTANCE instance, rulerOrientation orientation)
{
	HWND ruler;
	RECT rect;
	GetClientRect(parent, &rect);
	uint32_t width = rect.right - rect.left;
	uint32_t height = rect.bottom - rect.top;
	switch (orientation)
	{
	case vertical:
		ruler = CreateWindowEx(0, TEXT("Ruler"), TEXT("RulerVertical"), WS_CHILD | WS_VISIBLE, 0, RULER_THICKNESS, RULER_THICKNESS, height, parent, NULL, instance, NULL);
		SendMessage(ruler, WM_SET_RULER_ORIENTATION, vertical, NULL);
		break;
	case horizontal:
		ruler = CreateWindowEx(0, TEXT("Ruler"), TEXT("RulerHorizontal"), WS_CHILD | WS_VISIBLE, RULER_THICKNESS, 0, width, RULER_THICKNESS, parent, NULL, instance, NULL);
		SendMessage(ruler, WM_SET_RULER_ORIENTATION, horizontal, NULL);
		break;
	case corner:
		ruler = CreateWindowEx(0, TEXT("Ruler"), TEXT("RulerCorner"), WS_CHILD | WS_VISIBLE, 0, 0, RULER_THICKNESS, RULER_THICKNESS, parent, NULL, instance, NULL);
		SendMessage(ruler, WM_SET_RULER_ORIENTATION, corner, NULL);
		break;
	default:
		ruler = NULL;
		break;
	}
	return ruler;
}

static HWND createStatusBar(HWND parent, HINSTANCE instance, uint16_t width)
{
	HWND wnd = CreateWindowEx(
		0,                       // no extended styles
		STATUSCLASSNAME,         // name of status bar class
		(PCTSTR)NULL,			 // no text when first created
		WS_CHILD | WS_VISIBLE,   // creates a visible child window
		0, 0, 0, 0,              // ignores size and position
		parent,					 // handle to parent window
		NULL,					 // child window identifier
		instance,                // handle to application instance
		NULL);

	return wnd;
}

static void resizeStatusBar(UINT message, WPARAM wParam, LPARAM lParam, uint16_t width)
{
	if (statusBar)
	{
		SetWindowPos(statusBar, HWND_TOP, 0, 0, 0, 0, 0);
		SendMessage(statusBar, message, wParam, lParam);
		int edges[NUM_STATUS_PANES];
		for (uint8_t i = 0; i < NUM_STATUS_PANES; i++)
		{
			edges[i] = (width / NUM_STATUS_PANES) * (i+1);
		}
		
		SendMessage(statusBar, SB_SETPARTS, NUM_STATUS_PANES, (LPARAM)edges);
	}
}

void win32WriteToStatusBar(const char *text, uint8_t length, uint8_t panel)
{
	if (!statusBar) return;
	WCHAR t[256];
	MultiByteToWideChar(CP_ACP, 0, text, length, t, length);

	SendMessage(statusBar, SB_SETTEXT, panel, (LPARAM)t);
}


static void processCommands(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	// Parse the menu selections:
	switch (wmId)
	{
	case IDM_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		break;
	case IDM_EXIT:
		DestroyWindow(hWnd);
		break;
	case ID_FILE_NEW:
		printf("New\n");
		break;
	case ID_FILE_SAVE:
		printf("Save\n");
		break;
	case ID_FILE_SAVEAS:
		printf("Save As\n");
		break;
	case ID_VIEW_UNITSGRID:
		printf("Units and Grid...\n");
		DialogBox(hInst, MAKEINTRESOURCE(IDD_UNITSGRID), hWnd, UnitsGrid);
		break;
	case ID_VIEW_ZOOMIN:
		zoomIn();
		break;
	case ID_VIEW_ZOOMOUT:
		zoomOut();
		break;
	case ID_VIEW_ZOOM1:
		zoomOneToOne();
		break;
	default:
		DefWindowProc(hWnd, message, wParam, lParam);
	}

}


static void quitProgram()
{
	//TODO: Bug user to save unsaved work before quitting?
	vexClose();
	PostQuitMessage(0);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	uint32_t width = rect.right - rect.left;
	uint32_t height = rect.bottom - rect.top;

	HDC dc, newDC;
	PAINTSTRUCT paintStruct;
	HGDIOBJ oldBmp;
	HBITMAP bmp;

	cairo_surface_t *surface;
	cairo_t *cr;

	switch (message)
	{
		
	// ******************** Window Management Stuff **************************
	case WM_CREATE:
		vexInit();
		getDisplayInfo();
		rulerHorizontal = createRuler(hWnd, hInst, horizontal);
		rulerVertical = createRuler(hWnd, hInst, vertical);
		rulerCorner = createRuler(hWnd, hInst, corner);
		statusBar = createStatusBar(hWnd, hInst, width);
		break;
	case WM_SIZE:
		resizeRulers(message, wParam, lParam);
		resizeStatusBar(message, wParam, lParam, width);
		break;
	case WM_DESTROY:
		quitProgram(); 
		break;
	case WM_COMMAND:
		processCommands(hWnd, message, wParam, lParam);
		break;
	// ************************* Mouse Stuff *********************************
	case WM_MOUSEMOVE:
		mouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_LBUTTONDOWN:
		mouseDown(Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_LBUTTONUP:
		mouseUp(Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_MBUTTONDOWN:
		mouseDown(Middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_MBUTTONUP:
		mouseUp(Middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_RBUTTONDOWN:
		mouseDown(Right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), width, height);
		break;
	case WM_RBUTTONUP:
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

		vexPaintDrawingBoard(cr, width, height);
		cairo_destroy(cr);
		cairo_surface_destroy(surface);

		BitBlt(dc, 0, 0, width, height, newDC, rect.left, rect.top, SRCCOPY);
		SelectObject(newDC, oldBmp);
		DeleteDC(newDC);
		EndPaint(hWnd, &paintStruct);
		break;

	default:
		break;
	} //switch(message)

	return DefWindowProc(hWnd, message, wParam, lParam);

}



// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Called by external functions to trigger a redraw of main program window
void win32RedrawScreen()
{
	HWND wnd = FindWindow(szWindowClass, szTitle);
	InvalidateRect(wnd, NULL, NULL);
	//RedrawWindow(wnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);
}

void win32RedrawRulers()
{
	if(rulerHorizontal) InvalidateRect(rulerHorizontal, NULL, NULL);//RedrawWindow(rulerHorizontal, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_INTERNALPAINT);//InvalidateRect(rulerHorizontal, NULL, NULL);
	if(rulerVertical) InvalidateRect(rulerVertical, NULL, NULL);//RedrawWindow(rulerVertical, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_INTERNALPAINT);
}
