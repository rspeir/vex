#ifndef _vex_h
#define _vex_h

#include "resource.h"
#include "stdint.h"
#include "cairo-win32.h"

#define NUM_STATUS_PANES 6

#define MIN_GRID_SPACING 1
#define MAX_GRID_SPACING 32000

#define MIN_GRID_SUBDIVISIONS 0
#define MAX_GRID_SUBDIVISIONS 1000

typedef enum { Left = 0x01, Middle = 0x02, Right = 0x04 } clickType;  // allow for bitwise & and | so we can store combinations of clicks
typedef enum { millimeters, centimeters, meters, points, picas, inches, feet} unitType;
typedef enum { undefined, vertical, horizontal, corner } rulerOrientation;

void vexInit();
void vexClose();

void mouseMove(uint16_t x, uint16_t y, uint16_t winWidth, uint16_t winHeight);
void mouseDown(clickType click, uint16_t x, uint16_t y, uint16_t winWidth, uint16_t winHeight);
void mouseUp(clickType click, uint16_t x, uint16_t y, uint16_t winWidth, uint16_t winHeight);
void mouseWheel(int16_t delta, uint16_t x, uint16_t y, uint16_t winWidth, uint16_t winHeight);
void zoomOneToOne();
void zoomIn();
void zoomOut();

double convertUnits(double measurement, unitType in, unitType out);

void writeToStatusBar(const char *text, uint8_t panel);

void vexPaintRuler(cairo_t *cr, int32_t width, int32_t height, rulerOrientation orientation);
void vexPaintDrawingBoard(cairo_t *cr, int32_t width, int32_t height);

typedef struct
{
	uint32_t x;
	uint32_t y;
	int32_t scroll;
	uint8_t buttons;
} mouseState;

typedef struct
{
	uint16_t dpi;

	unitType currUnitType;
	uint32_t gridSpacing;
	uint32_t gridSubdivisions;

	mouseState prevMouseState;
	mouseState currMouseState;

	//Apply zoom before applying pan!
	double zoom;
	int32_t panX;
	int32_t panY;

} programState;

typedef struct
{
	int32_t xTranslation;
	int32_t yTranslation;
	double zoom;
} canvasTransform;


programState *vexGetProgramState();
void vexSetProgramStateGridSpacing(uint32_t gridSpacing);
void vexSetProgramStateGridSubdivisions(uint32_t gridSubdivisions);
void vexSetProgramStateUnits(unitType units);
void vexSetProgramStateDisplayInfo(uint32_t dpi);


#endif