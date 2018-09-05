#ifndef _vex_h
#define _vex_h

#include "resource.h"
#include "stdint.h"
#include "cairo-win32.h"

#define NUM_STATUS_PANES 6

#define RULER_THICKNESS 24

#define MIN_GRID_SPACING 1
#define MAX_GRID_SPACING 32000

#define MIN_GRID_SUBDIVISIONS 0
#define MAX_GRID_SUBDIVISIONS 1000

typedef enum { Left = 0x01, Middle = 0x02, Right = 0x04 } clickType;  // allow for bitwise & and | so we can store combinations of clicks

//If you add a new unit type, be sure to put its name and character representation
//at the same location in the respective enum and char array so it doesn't break
//the unitTypeString function.
typedef enum { millimeters, centimeters, meters, points, picas, inches, feet} unitType;
static const char *unitNames[7] = { "mm", "cm", "m", "pt", "pc", "in", "ft" };	
typedef enum { undefined, vertical, horizontal, corner } rulerOrientation;

typedef enum {nulltool, pentool, pointselect, groupselect, bezier } toolType;

void vexInit();
void vexClose();

const char *unitTypeString(unitType unit);
void mouseMove(int32_t x, int32_t y, uint16_t winWidth, uint16_t winHeight);
void mouseDown(clickType click, int32_t x, int32_t y, uint16_t winWidth, uint16_t winHeight);
void mouseUp(clickType click, int32_t x, int32_t y, uint16_t winWidth, uint16_t winHeight);
void mouseWheel(int32_t delta, int32_t x, int32_t y, uint16_t winWidth, uint16_t winHeight);
void zoomOneToOne();
void zoomIn(int32_t x, int32_t y, uint16_t width, uint16_t height);
void zoomOut(int32_t x, int32_t y, uint16_t width, uint16_t height);

double convertUnits(double measurement, unitType in, unitType out);

void writeToStatusBar(const char *text, uint8_t panel);

void vexPaintRuler(cairo_t *cr, int32_t width, int32_t height, rulerOrientation orientation);
void vexPaintCanvasBackground(cairo_t *cr, int32_t width, int32_t height);
void vexPaintDrawingObjects(cairo_t *cr, int32_t width, int32_t height);


typedef struct
{
	bool selected;
	double x;
	double y;
} vexPoint;

typedef struct
{
	bool selected;
	vexPoint anchorA;
	vexPoint handleA;
	vexPoint anchorB;
	vexPoint handleB;
} vexBezier;

typedef struct pointList
{
	vexPoint *point;
	pointList *next;
} pointList;

typedef struct
{
	pointList *points;
} vexDrawingObjects;

typedef struct
{
	double x;
	double y;
	int32_t scroll;
	uint8_t buttons;
} mouseState;

typedef struct
{
	uint16_t dpi;

	unitType currUnitType;
	uint32_t gridSpacing;
	uint32_t gridSubdivisions;

	toolType currTool;

	mouseState prevMouseState;
	mouseState currMouseState;

	vexDrawingObjects drawing;

	//Apply zoom before applying pan!
	double zoom;
	double panX;
	double panY;

} programState;




programState *vexGetProgramState();
void vexSetProgramStateGridSpacing(uint32_t gridSpacing);
void vexSetProgramStateGridSubdivisions(uint32_t gridSubdivisions);
void vexSetProgramStateUnits(unitType units);
void vexSetProgramStateDisplayInfo(uint32_t dpi);


#endif