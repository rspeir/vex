#include "stdafx.h"
#include "Vex.h"
#include "Vex_Win32Platform.h"
#include "Vex_Win32Rulers.h"
#include "Vex_Win32Canvas.h"
#include <stdio.h>
#include <math.h>

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
	globalProgramState->currTool = pentool;
	globalProgramState->drawing.points = NULL;
	zoomOneToOne();
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

//TODO: Free sub-structures in globalProgramState before freeing globalProgramState
void vexClose()
{
	free(globalProgramState);
}


void vexScreenToCanvas(vexPoint *screenPoint)
{
	screenPoint->x = (screenPoint->x - globalProgramState->panX) / globalProgramState->zoom;
	screenPoint->y = (screenPoint->y - globalProgramState->panY) / globalProgramState->zoom;
}

void vexCanvasToScreen(vexPoint *canvasPoint)
{
	canvasPoint->x = (canvasPoint->x * globalProgramState->zoom) + globalProgramState->panX;
	canvasPoint->y = (canvasPoint->y * globalProgramState->zoom) + globalProgramState->panY;
}

double vexScreenToCanvasX(double x)
{
	return (x - globalProgramState->panX) / globalProgramState->zoom;
}

double vexScreenToCanvasY(double y)
{
	return (y - globalProgramState->panY) / globalProgramState->zoom;
}

double vexCanvasToScreenX(double x)
{
	return (x * globalProgramState->zoom) + globalProgramState->panX;
}

double vexCanvasToScreenY(double y)
{
	return (y * globalProgramState->zoom) + globalProgramState->panY;
}



void addPoint(float x, float y)
{
	vexPoint *newPoint = (vexPoint *)malloc(sizeof(vexPoint));

	newPoint->x = x;
	newPoint->y = y;
	newPoint->selected = TRUE;

	vexScreenToCanvas(newPoint);

	pointList *newPointList = (pointList *)malloc(sizeof(pointList));
	newPointList->point = newPoint;
	newPointList->next = NULL;

	if (globalProgramState->drawing.points == NULL)
	{
		globalProgramState->drawing.points = newPointList;
	}
	else
	{
		pointList *currentPoint = globalProgramState->drawing.points;
		while (currentPoint->next != NULL)
		{
			currentPoint = currentPoint->next;
		}
		if (currentPoint->point->selected == TRUE) currentPoint->point->selected = FALSE;
		currentPoint->next = newPointList;
	}
}

void mouseMove(int32_t x, int32_t y, uint16_t winWidth, uint16_t winHeight)
{
	char buff[256];
	
	globalProgramState->prevMouseState.x = globalProgramState->currMouseState.x;
	globalProgramState->prevMouseState.y = globalProgramState->currMouseState.y;
	globalProgramState->currMouseState.x = x;
	globalProgramState->currMouseState.y = y;

	// Right button drag = pan display area
	if (globalProgramState->currMouseState.buttons & Right)
	{
		
		globalProgramState->panX += ((double)x - (double)globalProgramState->prevMouseState.x);
		globalProgramState->panY += ((double)y - (double)globalProgramState->prevMouseState.y);
	
		sprintf(buff, "Pan %f, %f", globalProgramState->panX, globalProgramState->panY);
		writeToStatusBar(buff, 1);
		win32RedrawScreen();
		
	}

	win32RedrawRulers();

	sprintf(buff, "X: %d Y: %d", x, y);
	writeToStatusBar(buff, 0);
}

void mouseDown(clickType click, int32_t x, int32_t y, uint16_t winWidth, uint16_t winHeight)
{
	globalProgramState->currMouseState.buttons |= click;
	globalProgramState->currMouseState.x = x;
	globalProgramState->currMouseState.y = y;

	char buff[256];
	sprintf(buff, "Click at: %d, %d. Button %d clicked.", x, y, globalProgramState->currMouseState.buttons);
	writeToStatusBar(buff, 0);
}

void mouseUp(clickType click, int32_t x, int32_t y, uint16_t winWidth, uint16_t winHeight)
{
	globalProgramState->currMouseState.buttons &= ~click;

	if (click == Left && globalProgramState->prevMouseState.buttons | Left)
	{
		addPoint((float)x, (float)y);
	}
	win32RedrawScreen();
}

void mouseWheel(int32_t delta, int32_t x, int32_t y, uint16_t winWidth, uint16_t winHeight)
{
	if (delta > 1)
	{
		zoomIn(x, y, winWidth, winHeight);
	}
	if (delta < 1)
	{
		zoomOut(x, y, winWidth, winHeight);
	}
	char msg[255];
	sprintf(msg, "Zoom: %f%%", (globalProgramState->zoom) * 100);
	writeToStatusBar(msg, 2);
	sprintf(msg, "Pan %f, %f", globalProgramState->panX, globalProgramState->panY);
	writeToStatusBar(msg, 1);
	win32RedrawScreen();
	win32RedrawRulers();
}

void zoomIn(int32_t x, int32_t y, uint16_t width, uint16_t height)
{
	if (globalProgramState->zoom < 10.0)
	{
		globalProgramState->panX -= (width/2) * 0.1;
		globalProgramState->panY -= (height/2) * 0.1;
		globalProgramState->zoom += 0.1f;	
	}
}

void zoomOut(int32_t x, int32_t y, uint16_t width, uint16_t height)
{
	if (globalProgramState->zoom > 0.11)
	{
		globalProgramState->panX += (width/2) * .1;
		globalProgramState->panY += (height/2) * .1;
		globalProgramState->zoom -= 0.1f;
	}
}

void zoomOneToOne()
{
	globalProgramState->zoom = 1.0;
	globalProgramState->panX = 0;
	globalProgramState->panY = 0;
	char msg[255];
	sprintf(msg, "Zoom: %f%%", (globalProgramState->zoom) * 100);
	writeToStatusBar(msg, 2);
	win32RedrawScreen();
	win32RedrawRulers();
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

const char *unitTypeString(unitType unit)
{
	return unitNames[unit];
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


void vexPaintRuler(cairo_t *cr, int32_t width, int32_t height, rulerOrientation orientation)
{
	double mouseX = globalProgramState->currMouseState.x;
	double mouseY = globalProgramState->currMouseState.y;
	
	const double dashes[] = { 2.0, 2.0 };
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_line_width(cr, 1.0);

	double pixelsPerGridLine = (double)globalProgramState->gridSpacing / globalProgramState->gridSubdivisions;
	double minX = vexScreenToCanvasX(0);
	double maxX = vexScreenToCanvasX(width);
	double minY = vexScreenToCanvasY(0);
	double maxY = vexScreenToCanvasY(height)+RULER_THICKNESS;
	int lineCount;

	switch (orientation)
	{
	case horizontal:
		// Background color for ruler
		cairo_set_source_rgb(cr, 0.87, 0.90, 0.80);
		cairo_rectangle(cr, 0, 0, width, height);
		cairo_fill(cr);

		// Mouse cursor position indicator
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to(cr, mouseX-RULER_THICKNESS, 0);
		cairo_line_to(cr, mouseX-RULER_THICKNESS, RULER_THICKNESS);
		cairo_stroke(cr);
		
		// Ruler boundary line
		cairo_set_source_rgb(cr, 0.58, 0.65, 0.52);
		cairo_move_to(cr, 0, height);
		cairo_line_to(cr, width, height);
		cairo_stroke(cr);
		
		// Measurement lines
		cairo_set_source_rgb(cr, 0.68, 0.75, 0.62);
		lineCount = 0;
		for (double x = 0; x < maxX; x += pixelsPerGridLine)
		{
			cairo_move_to(cr, vexCanvasToScreenX(x)-RULER_THICKNESS, height);
			if (lineCount % globalProgramState->gridSubdivisions == 0)
			{
				cairo_line_to(cr, vexCanvasToScreenX(x) - RULER_THICKNESS, height - RULER_THICKNESS);
			}
			else
			{
				cairo_line_to(cr, vexCanvasToScreenX(x) - RULER_THICKNESS, height - RULER_THICKNESS / 2);
			}
			cairo_stroke(cr);
			lineCount++;
		}
		lineCount = 0;
		for (double x = 0; x > minX; x -= pixelsPerGridLine)
		{
			cairo_move_to(cr, vexCanvasToScreenX(x) - RULER_THICKNESS, height);
			if (lineCount % globalProgramState->gridSubdivisions == 0)
			{
				cairo_line_to(cr, vexCanvasToScreenX(x) - RULER_THICKNESS, height - RULER_THICKNESS);
			}
			else
			{
				cairo_line_to(cr, vexCanvasToScreenX(x) - RULER_THICKNESS, height - RULER_THICKNESS / 2);
			}
			cairo_stroke(cr);
			lineCount++;
		}


		break;
	case vertical:
		// Background color
		cairo_set_source_rgb(cr, 0.87, 0.90, 0.80);
		cairo_rectangle(cr, 0, 0, width, height);
		cairo_fill(cr);
		// Mouse cursor position indicator
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to(cr, 0, mouseY-RULER_THICKNESS);
		cairo_line_to(cr, RULER_THICKNESS, mouseY-RULER_THICKNESS);
		cairo_stroke(cr);

		// Measurement lines
		lineCount = 0;
		cairo_set_source_rgb(cr, 0.68, 0.75, 0.62);
		for (double y = 0; y < maxY; y += pixelsPerGridLine)
		{
			cairo_move_to(cr, RULER_THICKNESS, vexCanvasToScreenY(y)-RULER_THICKNESS);
			if (lineCount % globalProgramState->gridSubdivisions == 0)
			{
				cairo_line_to(cr, 0, vexCanvasToScreenY(y)-RULER_THICKNESS);
			}
			else
			{
				cairo_line_to(cr, RULER_THICKNESS / 2, vexCanvasToScreenY(y)-RULER_THICKNESS);
			}
			cairo_stroke(cr);
			lineCount++;
		}
		lineCount = 0;
		for (double y = 0; y > minY; y -= pixelsPerGridLine)
		{
			cairo_move_to(cr, RULER_THICKNESS, vexCanvasToScreenY(y) - RULER_THICKNESS);
			if (lineCount % globalProgramState->gridSubdivisions == 0)
			{
				cairo_line_to(cr, 0, vexCanvasToScreenY(y) - RULER_THICKNESS);
			}
			else
			{
				cairo_line_to(cr, RULER_THICKNESS / 2, vexCanvasToScreenY(y) - RULER_THICKNESS);
			}
			cairo_stroke(cr);
			lineCount++;
		}

		// Ruler Boundary
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

		char *unitLabel;

		switch (globalProgramState->currUnitType)
		{
		case millimeters:
			unitLabel = "mm";
			break;
		case centimeters:
			unitLabel = "cm";
			break;
		case meters:
			unitLabel = "m";
			break;
		case points:
			unitLabel = "pt";
			break;
		case picas:
			unitLabel = "pc";
			break;
		case inches:
			unitLabel = "in";
			break;
		case feet:
			unitLabel = "ft";
			break;
		default:
			unitLabel = "??";
			break;
		}

		cairo_text_extents(cr, unitLabel, &extents);

		cairo_move_to(cr, width / 2 - extents.width / 2, height-extents.height);
		cairo_show_text(cr, unitLabel);
		break;
	}
	//cairo_status_t cstat = cairo_status(cr);
	//const char *statstring;
	//statstring = cairo_status_to_string(cstat);
	//printf("Cairo Status: %s\n", statstring);
}


void vexPaintDrawingObjects(cairo_t *cr, int32_t width, int32_t height)
{

	//Start by drawing points:
	// convert from canvas coords to screen coords
	pointList *currPoint = globalProgramState->drawing.points;
	while (currPoint != NULL)
	{
		vexPoint point = { false, currPoint->point->x, currPoint->point->y };
		vexCanvasToScreen(&point);
		cairo_move_to(cr, point.x, point.y);
		cairo_arc(cr, point.x, point.y, 2, 0, 2*3.141592654);
		cairo_set_source_rgb(cr, 0, 0, 0);
	
		currPoint = currPoint->next;
	}
	cairo_fill(cr);

}


// do we really need to do this?
// NO.
#if 0
double sizeInPixels(unitType unit)
{
	double res = globalProgramState->dpi * convertUnits(1.0, unit, inches) * globalProgramState->zoom;
	return res;
}
#endif


void vexPaintCanvasBackground(cairo_t *cr, int32_t width, int32_t height)
{

	//Background
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_set_source_rgb(cr, 0.92, 0.95, 0.85);
	cairo_fill(cr);
	
	double pixelsPerGridLine = (double)globalProgramState->gridSpacing / globalProgramState->gridSubdivisions;

	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_line_width(cr, 1.0);

	int lineCount = 0;
	for (double x = 0; x < width; x+= pixelsPerGridLine)
	{
		if (lineCount % globalProgramState->gridSubdivisions == 0)
		{
			cairo_set_source_rgb(cr, 0.68, 0.75, 0.62);
		}
		else
		{
			cairo_set_source_rgb(cr, 0.78, 0.85, 0.72);
		}
		cairo_move_to(cr, vexCanvasToScreenX(x), vexCanvasToScreenY(0));
		cairo_line_to(cr, vexCanvasToScreenX(x), vexCanvasToScreenY(height));
		lineCount++;
		cairo_stroke(cr);
	}
	lineCount = 0;
	for (double y = 0; y < height; y += pixelsPerGridLine)
	{
		if (lineCount % globalProgramState->gridSubdivisions == 0)
		{
			cairo_set_source_rgb(cr, 0.68, 0.75, 0.62);
		}
		else
		{
			cairo_set_source_rgb(cr, 0.78, 0.85, 0.72);
		}
		cairo_move_to(cr, vexCanvasToScreenX(0), vexCanvasToScreenY(y));
		cairo_line_to(cr, vexCanvasToScreenX(width), vexCanvasToScreenY(y));
		lineCount++;
		cairo_stroke(cr);
	}
}
