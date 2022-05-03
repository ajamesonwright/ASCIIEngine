#ifndef ASCIIENGINE_MAIN_WINDOW_H_
#define ASCIIENGINE_MAIN_WINDOW_H_

#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <vector>
#include "geometry.h"
#include "input.h"

// shorten name, make it easier to use
#define MW main_window

class Renderer;

namespace main_window {

	// execution state
	int run_state_;
	int draw_mode_;

	// current mouseover panel
	int current_panel_ = -1;

	Renderer* renderer_ = nullptr;
	Input* input_ = nullptr;

	// Window/exe properties
	const uint16_t window_starting_height_ = 600;
	const uint16_t window_starting_width_ = 1620;
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
	Ray2d camera;
	std::vector<Geometry*> geometry_queue;

	MSG event_message;

	Renderer* GetRenderer();
	Rect* GetDrawAreaRect(int panel = 2);

	bool GetRunningState();
	void SetRunningState(int p_run_state);

	void SetDrawMode(int p_draw_mode);

	int GetCursorFocus(Point2d p);

	void* FindMemoryHandle(Geometry* g);

	void SetWindowHeight(uint16_t p_height);
	uint16_t GetWindowHeight();
	void SetWindowWidth(uint16_t p_width);
	uint16_t GetWindowWidth();
	void SetWindowOffsetX(uint16_t p_offset);
	uint16_t GetWindowOffsetX();
	void SetWindowOffsetY(uint16_t p_offset);
	uint16_t GetWindowOffsetY();
	void SetMTCOffsetX(uint8_t p_offset_x);
	uint8_t GetMTCOffsetX();
	void SetMTCOffsetY(uint8_t p_offset_y);
	uint8_t GetMTCOffsetY();

	Rect& GetMainWindowRect();
	void SetMainWindowRect(HWND hwnd, Rect* rect);
	Rect& GetDrawRect();
	void SetDrawRect(HWND hwnd, Rect* rect);

	void ConditionMouseCoords(Point2d& p);
	void ConditionMouseCoords(POINT& p);
};

enum run_state {
	STOPPED,
	RUNNING,
};

enum draw_mode {
	D_TRI,
	D_RECT,
	D_QUAD,
	D_CIRCLE,
	
	D_DRAW_MODE_SIZE,
};

#endif