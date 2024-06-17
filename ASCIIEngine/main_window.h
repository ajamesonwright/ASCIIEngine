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
	int run_state_;
	int draw_mode_;

	// current mouseover panel
	int current_panel_ = -1;

	Renderer* renderer_ = nullptr;
	Input* input_ = nullptr;

	// Window/exe properties
	const uint16_t window_starting_height_ = 600;
	const uint16_t window_starting_width_ = 1920;
	const uint8_t window_starting_x_ = 100;
	const uint8_t window_starting_y_ = 200;
	const uint8_t border_width_ = 20;
	uint16_t window_height_ = 600;								// current client area height
	uint16_t window_width_ = 800;								// current client area width
	uint16_t xPos_ = 100;										// window top left x
	uint16_t yPos_ = 200;										// window top left y
	uint8_t main_to_client_offset_x_ = 0;
	uint8_t main_to_client_offset_y_ = 0;

	Rect main_rect, draw_rect;
	Point2d geo_start, geo_end;
	Camera* camera;
	std::vector<Geometry*> geometryQueue;
	Quadtree* qt;

	MSG event_message;

	Renderer* getRenderer();
	Rect* getDrawAreaPanel(int panel);

	bool getRunningState();
	void setRunningState(int p_run_state);
	void setDrawMode(int p_draw_mode);
	int getCursorFocus(Point2d p);

	void addGeometry(Geometry* g);
	void removeGeometry(Geometry* g);
	void simulateFrame(float s_per_frame);

	void* findMemoryHandle(Geometry* g);

	void setWindowHeight(uint16_t p_height);
	uint16_t getWindowHeight();
	void setWindowWidth(uint16_t p_width);
	uint16_t getWindowWidth();
	void setWindowOffsetX(uint16_t p_offset);
	uint16_t getWindowOffsetX();
	void setWindowOffsetY(uint16_t p_offset);
	uint16_t getWindowOffsetY();
	void setMTCOffsetX(uint8_t p_offset_x);
	uint8_t getMTCOffsetX();
	void setMTCOffsetY(uint8_t p_offset_y);
	uint8_t getMTCOffsetY();

	Rect& getMainWindowRect();
	void setMainWindowRect(HWND hwnd, Rect* rect);
	Rect& getDrawRect();
	void setDrawRect(HWND hwnd, Rect* rect);

	void conditionMouseCoords(Point2d& p);
	void conditionMouseCoords(POINT& p);
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