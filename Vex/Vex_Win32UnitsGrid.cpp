#include "stdafx.h"
#include "Vex_Win32UnitsGrid.h"
#include "Vex.h"
#include <stdio.h>
#include <windowsx.h>



void populateEditBox(HWND box, uint16_t number)
{
	char asciiText[255];
	TCHAR wideText[255];
	sprintf(asciiText, "%d", number);
	MultiByteToWideChar(CP_ACP, 0, asciiText, -1, wideText, 255);
	Edit_SetText(box, (LPCWSTR)wideText);
}

void populateUnitsLabel(HWND label, unitType unit)
{
	//const char *unitNames[7] = { "mm", "cm", "m", "pt", "pc", "in", "ft" };
	SetWindowTextA(label, unitTypeString(unit));
}

void validateEditBox(HWND wnd, uint16_t minval, uint16_t maxval)
{
	printf("Validating edit box.\n");
	//uint16_t value;
	//GetDlgItemInt(wnd, )
}



// Message handler for Units and Grid dialog box.
INT_PTR CALLBACK UnitsGrid(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static unitType units;
	static uint16_t gridSpacing, gridSubdivisions;
	int unitRadioButtons[7] = { IDC_RADIO1, IDC_RADIO2, IDC_RADIO3, IDC_RADIO4, IDC_RADIO5, IDC_RADIO6, IDC_RADIO7 };
	
	HWND gridSpacingEditBox, gridSubdivisionsEditBox, unitLabel, okButton;
	gridSpacingEditBox = GetDlgItem(hDlg, IDC_EDIT_GRIDSPACING);
	gridSubdivisionsEditBox = GetDlgItem(hDlg, IDC_EDIT_GRIDSUBDIVISIONS);
	unitLabel = GetDlgItem(hDlg, IDC_UNITLABEL);
	okButton = GetDlgItem(hDlg, IDOK);
	switch (message)
	{
		case WM_INITDIALOG:
			units = vexGetProgramState()->currUnitType;
			gridSpacing = vexGetProgramState()->gridSpacing;
			gridSubdivisions = vexGetProgramState()->gridSubdivisions;
			populateEditBox(gridSpacingEditBox, gridSpacing);
			populateEditBox(gridSubdivisionsEditBox, gridSubdivisions);
			populateUnitsLabel(unitLabel, units);
			
			CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO7, unitRadioButtons[units]);
			break;
		
		case WM_UPDATEDLG:
			populateUnitsLabel(unitLabel, (unitType)wParam);
			//gridSpacing = convertUnits(gridSpacing, units, (unitType)wParam);
			//populateEditBox(gridSpacingEditBox, gridSpacing);
			units = (unitType)wParam;
			break;

		case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			vexSetProgramStateUnits(units);
			vexSetProgramStateGridSpacing(gridSpacing);
			vexSetProgramStateGridSubdivisions(gridSubdivisions);
			InvalidateRect(GetParent(hDlg), NULL, NULL);
			EndDialog(hDlg, LOWORD(wParam));
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			break;
		
		case IDC_EDIT_GRIDSPACING:
			if (HIWORD(wParam) == EN_UPDATE)
			{
				BOOL translated = false;
				uint32_t num = GetDlgItemInt(hDlg, IDC_EDIT_GRIDSPACING, &translated, false);
				if (translated && num >= MIN_GRID_SPACING && num < MAX_GRID_SPACING)
				{
					Button_Enable(okButton, true);
					gridSpacing = (uint16_t)num;
				}
				else
				{
					Button_Enable(okButton, false);
				}
			}
			break;
		case IDC_EDIT_GRIDSUBDIVISIONS:
			if (HIWORD(wParam) == EN_UPDATE)
			{
				BOOL translated = false;
				uint32_t num = GetDlgItemInt(hDlg, IDC_EDIT_GRIDSUBDIVISIONS, &translated, false);
				if (translated && num >= MIN_GRID_SUBDIVISIONS && num < MAX_GRID_SUBDIVISIONS)
				{
					Button_Enable(okButton, true);
					gridSubdivisions = (uint16_t)num;
				}
				else
				{
					Button_Enable(okButton, false);
				}
			}
			break;
		case IDC_RADIO1:	//Millimeters
			SendMessage(hDlg, WM_UPDATEDLG, millimeters, 0);
			break;
		case IDC_RADIO2:	//Centimeters
			SendMessage(hDlg, WM_UPDATEDLG, centimeters, 0);
			break;
		case IDC_RADIO3:	//Meters
			SendMessage(hDlg, WM_UPDATEDLG, meters, 0);
			break;
		case IDC_RADIO4:	//Points
			SendMessage(hDlg, WM_UPDATEDLG, points, 0);
			break;
		case IDC_RADIO5:	//Picas
			SendMessage(hDlg, WM_UPDATEDLG, picas, 0);
			break;
		case IDC_RADIO6:	//Inches
			SendMessage(hDlg, WM_UPDATEDLG, inches, 0);
			break;
		case IDC_RADIO7:	//Feet
			SendMessage(hDlg, WM_UPDATEDLG, feet, 0);
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
