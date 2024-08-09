#ifndef ASCIIENGINE_MAIN_WINDOW_H_
#define ASCIIENGINE_MAIN_WINDOW_H_

#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <vector>
#include "geometry.h"
#include "input.h"
#include "quadtree.h"

// shorten name, make it easier to use
#define MW MainWindow

class Renderer;

namespace MainWindow {

	// execution state
	int runState;
	int drawMode;

	// draw shape outline flag
	int outlineType = 0;

	// current mouseover panel
	int currentPanel = -1;

	Renderer* renderer = nullptr;
	Input* input = nullptr;

	// Window/exe properties
	const uint16_t windowStartingHeight = 600;
	const uint16_t windowStartingWidth = 1920;
	const uint8_t windowStartingX = 100;
	const uint8_t windowStartingY = 200;
	const uint8_t borderWidth = 20;
	uint16_t windowHeight = 600;								// current client area height
	uint16_t windowWidth = 800;								// current client area width
	uint16_t xPos = 100;										// window top left x
	uint16_t yPos = 200;										// window top left y
	uint8_t mainToClientOffsetX = 0;
	uint8_t mainToClientOffsetY = 0;

	Rect mainRect, drawRect;
	Point2d geoStart, geoEnd;
	Line highlightLine;
	Camera* camera;
	std::vector<Geometry*> geometryQueue;
	Quadtree* qt;

	MSG eventMessage;

	Renderer* getRenderer();
	Rect* getDrawAreaPanel(int panel);

	bool getRunningState();
	void setRunningState(int pRunState);
	void setDrawMode(int pDrawMode);
	int getCursorFocus(Point2d p);

	void addGeometry(Geometry* g);
	void removeGeometry(Geometry* g);
	void simulateFrame(float secondsPerFrame);

	void* findMemoryHandle(Geometry* g);

	void setWindowHeight(uint16_t height);
	uint16_t getWindowHeight();
	void setWindowWidth(uint16_t width);
	uint16_t getWindowWidth();
	void setWindowOffsetX(uint16_t offset);
	uint16_t getWindowOffsetX();
	void setWindowOffsetY(uint16_t offset);
	uint16_t getWindowOffsetY();
	void setMTCOffsetX(uint8_t offset);
	uint8_t getMTCOffsetX();
	void setMTCOffsetY(uint8_t offset);
	uint8_t getMTCOffsetY();

	Rect& getMainWindowRect();
	void setMainWindowRect(HWND hwnd, Rect* rect);
	Rect& getDrawRect();
	void setDrawRect(HWND hwnd, Rect* rect);

	void conditionMouseCoords(Point2d& p);
	void conditionMouseCoords(POINT& p);

	bool canDrawHightlightLine();
	void drawHighlightLine();
};

enum RunState {
	STOPPED,
	RUNNING,
};

enum DrawMode {
	D_LINE,
	D_TRI,
	D_RECT,
	D_QUAD,
	D_CIRCLE,
	
	D_DRAW_MODE_SIZE,
};

#endif