#ifndef ASCIIENGINE_MAIN_WINDOW_H_
#define ASCIIENGINE_MAIN_WINDOW_H_

#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <vector>
#include "Geometry.h"
#include "Input.h"
#include "Quadtree.h"

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
	Camera camera;
	std::vector<Geometry*> geometry_queue;
	Quadtree* qt;

	MSG event_message;

	Renderer* GetRenderer();
	Rect* GetDrawAreaPanel(int panel);

	bool GetRunningState();
	void setRunningState(int p_run_state);
	void setDrawMode(int p_draw_mode);
	int GetCursorFocus(Point2d p);

	void AddGeometry(Geometry* g);
	void RemoveGeometry(Geometry* g);
	void SimulateFrame(float s_per_frame);

	void* FindMemoryHandle(Geometry* g);

	void setWindowHeight(uint16_t p_height);
	uint16_t GetWindowHeight();
	void setWindowWidth(uint16_t p_width);
	uint16_t GetWindowWidth();
	void setWindowOffsetX(uint16_t p_offset);
	uint16_t GetWindowOffsetX();
	void setWindowOffsetY(uint16_t p_offset);
	uint16_t GetWindowOffsetY();
	void setMTCOffsetX(uint8_t p_offset_x);
	uint8_t GetMTCOffsetX();
	void setMTCOffsetY(uint8_t p_offset_y);
	uint8_t GetMTCOffsetY();

	Rect& GetMainWindowRect();
	void setMainWindowRect(HWND hwnd, Rect* rect);
	Rect& GetDrawRect();
	void setDrawRect(HWND hwnd, Rect* rect);

	void ConditionMouseCoords(Point2d& p);
	void ConditionMouseCoords(POINT& p);
};

enum run_state {
	STOPPED,
	RUNNING,
};

enum draw_mode {
	D_LINE,
	D_TRI,
	D_RECT,
	D_QUAD,
	D_CIRCLE,
	
	D_DRAW_MODE_SIZE,
};

#endif