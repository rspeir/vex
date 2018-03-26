#include "stdafx.h"
#include "Vex.h"
#include "Vex_Win32Platform.h"
#include "Vex_Win32Rulers.h"
#include <stdio.h>
#include <math.h>

uint32_t paintRulersCallCount;

const char *unitNames[7] = { "millimeters", "centimeters", "meters", "points", "picas", "inches", "feet" };

static programState *globalProgramState;

static void printGlobalProgramState()
{
	printf("Global Program State:\n");
	
}


void vexInit()
{
	globalProgramState = (programState *)malloc(sizeof(programState));
	globalProgramState->currUnitType = points;
	globalProgramState->gridSpacing = 72;		// 72 points per inch = 1 major grid line every inch
	globalProgramState->gridSubdivisions = 4;   // minor grid line every 1/4 inch
	globalProgramState->dpi = 96;
	globalProgramState->panX = 0;
	globalProgramState->panY = 0;
	globalProgramState->currMouseState.buttons = 0;
	zoomOneToOne();
	//globalProgramState->zoom = 1.0;
	paintRulersCallCount = 0;
}

programState *vexGetProgramState()
{
	return globalProgramState;
}

void vexSetProgramStateGridSpacing(uint32_t gridSpacing)
{
	printf("Setting grid spacing to %d.\n", gridSpacing);
	globalProgramState->gridSpacing = gridSpacing;
}

void vexSetProgramStateGridSubdivisions(uint32_t subdivisions)
{
	printf("Setting grid subdivisions to %d.\n", subdivisions);
	globalProgramState->gridSubdivisions = subdivisions;
}

void vexSetProgramStateUnits(unitType units)
{
	printf("Setting units to %s.\n", unitNames[units]);
	globalProgramState->currUnitType = units;
}

void vexSetProgramStateDisplayInfo(uint32_t dpi)
{
	globalProgramState->dpi = dpi;
}

void vexClose()
{
	free(globalProgramState);
}

void mouseMove(uint16_t x, uint16_t y, uint16_t winWidth, uint16_t winHeight)
{
	char buff[256];
	
	globalProgramState->prevMouseState.x = globalProgramState->currMouseState.x;
	globalProgramState->prevMouseState.y = globalProgramState->currMouseState.y;
	globalProgramState->currMouseState.x = x;
	globalProgramState->currMouseState.y = y;
	paintRulersCallCount++;
	win32RedrawRulers();

	// Right button drag = pan display area
	if (globalProgramState->currMouseState.buttons & Right)
	{
		
		globalProgramState->panX += (x - globalProgramState->prevMouseState.x);
		globalProgramState->panY += (y - globalProgramState->prevMouseState.y);
	
		sprintf(buff, "Panning to %d, %d", globalProgramState->panX, globalProgramState->panY);
		writeToStatusBar(buff, 1);
		win32RedrawScreen();
		
	}

	sprintf(buff, "X: %d Y: %d", x, y);
	writeToStatusBar(buff, 0);
}

void mouseDown(clickType click, uint16_t x, uint16_t y, uint16_t winWidth, uint16_t winHeight)
{
	globalProgramState->currMouseState.buttons |= click;
	globalProgramState->currMouseState.x = x;
	globalProgramState->currMouseState.y = y;

	char buff[256];
	sprintf(buff, "Click at: %d, %d. Button %d clicked.", x, y, globalProgramState->currMouseState.buttons);
	writeToStatusBar(buff, 0);
}

void mouseUp(clickType click, uint16_t x, uint16_t y, uint16_t winWidth, uint16_t winHeight)
{
	globalProgramState->currMouseState.buttons &= ~click;
}

void mouseWheel(int16_t delta, uint16_t x, uint16_t y, uint16_t winWidth, uint16_t winHeight)
{
	if (delta > 1)
	{
		zoomIn();
	}
	if (delta < 1)
	{
		zoomOut();
	}
}

void zoomIn()
{
	if (globalProgramState->zoom < 10.0)
	{
		globalProgramState->zoom *= 1.05;
	}
	char msg[255];
	sprintf(msg, "Zoom: %f%%", (globalProgramState->zoom) * 100);
	writeToStatusBar(msg, 1);
	win32RedrawScreen();
}

void zoomOut()
{
	if (globalProgramState->zoom > 0.1)
	{
		globalProgramState->zoom *= 0.95;
	}
	char msg[255];
	sprintf(msg, "Zoom: %f%%", (globalProgramState->zoom) * 100);
	writeToStatusBar(msg, 1);
	win32RedrawScreen();
}

void zoomOneToOne()
{
	globalProgramState->zoom = 1.0;
	char msg[255];
	sprintf(msg, "Zoom: %f%%", (globalProgramState->zoom) * 100);
	writeToStatusBar(msg, 1);
	win32RedrawScreen();
}

double convertUnits(double measurement, unitType in, unitType out)
{
	double retval = measurement;
	switch (in)
	{
	case millimeters:
		switch (out)
		{
		case millimeters:
			retval = measurement;
			break;
		case centimeters:
			retval =  measurement * 0.1;
			break;
		case meters:
			retval =  measurement * .001;
			break;
		case points:
			retval =  measurement * 2.83465;
			break;
		case picas:
			retval =  measurement * 0.236221;
			break;
		case inches:
			retval =  measurement * 0.0393701;
			break;
		case feet:
			retval =  measurement * 0.00328084;
			break;
		}
	break;
	case centimeters:
		switch (out)
		{
		case millimeters:
			retval =  measurement * 10.0;
			break;
		case centimeters:
			retval =  measurement;
			break;
		case meters:
			retval = measurement * .01;
			break;
		case points:
			retval =  measurement * 28.3465;
			break;
		case picas:
			retval =  measurement * 2.36221;
			break;
		case inches:
			retval =  measurement * 0.393701;
			break;
		case feet:
			retval =  measurement * 0.0328084;
			break;
		}
	break;
	case meters:
		switch (out)
		{
		case millimeters:
			retval = measurement * 1000.0;
			break;
		case centimeters:
			retval = measurement * 100.0;
			break;
		case meters:
			retval = measurement;
			break;
		case points:
			retval = measurement * 2834.65;
			break;
		case picas:
			retval = measurement * 236.221;
			break;
		case inches:
			retval = measurement * 39.3701;
			break;
		case feet:
			retval = measurement * 3.28084;
			break;
		}
	break;
	case points:
		switch (out)
		{
		case millimeters:
			retval =  measurement * 0.352778;
			break;
		case centimeters:
			retval =  measurement * 0.0352778;
			break;
		case meters:
			retval = measurement * 0.000352778;
			break;
		case points:
			retval =  measurement;
			break;
		case picas:
			retval =  measurement / 12.0;
			break;
		case inches:
			retval =  measurement / 72.0;
			break;
		case feet:
			retval =  measurement / 864.0;
			break;
		}
	break;
	case picas:
		switch (out)
		{
		case millimeters:
			retval =  measurement * 4.23332388;
			break;
		case centimeters:
			retval =  measurement * 0.423332388;
			break;
		case meters:
			retval = measurement * .00423332388;
			break;
		case points:
			retval =  measurement * 12.0;
			break;
		case picas:
			retval =  measurement;
			break;
		case inches:
			retval =  measurement / 6.0;
			break;
		case feet:
			retval =  measurement / 12.0;
			break;
		}
	break;
	case inches:
		switch (out)
		{
		case millimeters:
			retval =  measurement * 25.4;
			break;
		case centimeters:
			retval =  measurement * 2.54;
			break;
		case meters:
			retval = measurement * 0.0254;
			break;
		case points:
			retval =  measurement * 72.0;
			break;
		case picas:
			retval =  measurement * 6.0;
			break;
		case inches:
			retval =  measurement;
			break;
		case feet:
			retval =  measurement / 12.0;
			break;
		}
	break;
	case feet:
		switch (out)
		{
		case millimeters:
			retval =  measurement * 304.8;
			break;
		case centimeters:
			retval =  measurement * 30.48;
			break;
		case meters:
			retval = measurement * 0.3048;
			break;
		case points:
			retval =  measurement * 864.0;
			break;
		case picas:
			retval =  measurement * 72.0;
			break;
		case inches:
			retval =  measurement * 12.0;
			break;
		case feet:
			retval =  measurement;
			break;
		}
	break;
	}
	return retval;
}


void writeToStatusBar(const char *text, uint8_t panel)
{
	if (!text) return;							//bail out if empty string
	if (panel > NUM_STATUS_PANES) return;		//bail out if trying to write to pane that doesn't exist

	char temptext[256];
	uint8_t length = 0;

	while (length <= 255 && text[length] != '\0')
	{
		temptext[length] = text[length];
		length++;
	}
	temptext[length] = '\0';

	win32WriteToStatusBar(temptext, length+1, panel);
}


double sizeInPixels(unitType unit)
{
	double res = globalProgramState->dpi * convertUnits(1.0, unit, inches) * globalProgramState->zoom;
	return res;
}


void vexPaintRuler(cairo_t *cr, int32_t width, int32_t height, rulerOrientation orientation)
{
	uint32_t mouseX = globalProgramState->currMouseState.x;
	uint32_t mouseY = globalProgramState->currMouseState.y;

	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_line_width(cr, 1.0);
	switch (orientation)
	{
	case horizontal:
		cairo_set_source_rgb(cr, 0.87, 0.90, 0.80);
		cairo_rectangle(cr, 0, 0, width, height);
		cairo_fill(cr);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to(cr, mouseX - RULER_THICKNESS, 0);
		cairo_line_to(cr, mouseX - RULER_THICKNESS, height);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.58, 0.65, 0.52);
		cairo_move_to(cr, 0, height);
		cairo_line_to(cr, width, height);
		cairo_stroke(cr);
		break;
	case vertical:
		cairo_set_source_rgb(cr, 0.87, 0.90, 0.80);
		cairo_rectangle(cr, 0, 0, width, height);
		cairo_fill(cr);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to(cr, 0, mouseY - RULER_THICKNESS);
		cairo_line_to(cr, RULER_THICKNESS, mouseY - RULER_THICKNESS);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.58, 0.65, 0.52);
		cairo_move_to(cr, width, 0);
		cairo_line_to(cr, width, height);
		cairo_stroke(cr);
		break;
	case corner:
		cairo_set_source_rgb(cr, 0.87, 0.90, 0.80);
		cairo_rectangle(cr, 0, 0, width, height);
		cairo_fill(cr);
		cairo_set_source_rgb(cr, 0.48, 0.55, 0.42);

		cairo_move_to(cr, 0, height);
		cairo_line_to(cr, width, height);
		cairo_move_to(cr, width, 0);
		cairo_line_to(cr, width, height);
		cairo_stroke(cr);

		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_select_font_face(cr, "Helvetica", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr, 11);

		cairo_text_extents_t extents;
		cairo_text_extents(cr, "mm", &extents);

		cairo_move_to(cr, width / 2 - extents.width / 2, height-extents.height);
		cairo_show_text(cr, "mm");
		break;

	}
}


void vexPaintDrawingBoard(cairo_t *cr, int32_t width, int32_t height)
{
	//Background
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_set_source_rgb(cr, 0.92, 0.95, 0.85);
	cairo_fill(cr);

	
	int32_t pixelsPerGridLine = (uint32_t)ceill((sizeInPixels(globalProgramState->currUnitType) * ((double)globalProgramState->gridSpacing / (double)globalProgramState->gridSubdivisions)));
	
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_source_rgb(cr, 0.78, 0.85, 0.72);
	cairo_set_line_width(cr, 1.0);

	cairo_translate(cr, globalProgramState->panX, globalProgramState->panY);
	int32_t panX = globalProgramState->panX;
	int32_t panY = globalProgramState->panY;

	if (pixelsPerGridLine > 2)
	{
//		uint32_t lineCount = 0;
		for (int32_t i = (0 - panX + (panX % pixelsPerGridLine)); i < (width - panX + (panX % pixelsPerGridLine)); i += pixelsPerGridLine)
		{
			cairo_move_to(cr, i, 0-globalProgramState->panY);
			cairo_line_to(cr, i, height-globalProgramState->panY);
//			lineCount++;
		}
//		lineCount = 0;
		for (int32_t i = (0 - panY + (panY % pixelsPerGridLine)); i < (height - panY + (panY % pixelsPerGridLine)); i += pixelsPerGridLine)
		{
			cairo_move_to(cr, 0-globalProgramState->panX, i);
			cairo_line_to(cr, width-globalProgramState->panX, i);
//			lineCount++;
		}
	}
	
	cairo_stroke(cr);
}
