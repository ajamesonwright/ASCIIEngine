#ifndef ASCIIENGINE_MAIN_WINDOW_H_
#define ASCIIENGINE_MAIN_WINDOW_H_

#include <Windows.h>
#include <stdint.h>

// shorten name, make it easier to use
#define MW main_window

class Renderer;

namespace main_window {

	int run_state_;

	struct Input {
		int mouse_X = 0;
		int mouse_Y = 0;
	} input_;

	// Window/exe properties
	Renderer* renderer = nullptr;
	const uint16_t window_starting_height_ = 600;
	const uint16_t window_starting_width_ = 800;
	const uint8_t window_starting_x_ = 100;
	const uint8_t window_starting_y_ = 200;
	const uint8_t panel_divider_width_ = 20;
	uint16_t window_height_ = 600;								// current client area height
	uint16_t window_width_ = 800;								// current client area width
	uint16_t xPos_ = 100;										// window top left x
	uint16_t yPos_ = 200;										// window top left y
	uint8_t main_to_client_offset_x = 0;
	uint8_t main_to_client_offset_y = 0;

	RECT main_rect, draw_rect;

	static bool print_debug_ = false;
	Renderer* GetRenderer();

	bool GetRunningState();
	void SetRunningState(int p_run_state);

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

	RECT& GetMainWindowRect();
	void SetMainWindowRect(RECT& rect);
	RECT& GetDrawRect();
	void SetDrawRect(RECT& rect);

	void ConditionMouse(POINT& p);
};

enum {
	STOPPED,
	RUNNING,
};

#endif