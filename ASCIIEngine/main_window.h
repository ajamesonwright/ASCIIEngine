#ifndef ASCIIENGINE_MAIN_WINDOW_H_
#define ASCIIENGINE_MAIN_WINDOW_H_

#include <Windows.h>

class Renderer;

class main_window {
	Renderer* renderer;

	int run_state_;

	struct Input {
		int mouse_X = 0;
		int mouse_Y = 0;
	} input_;

	// Window/exe properties
public:
	const UINT window_starting_height_ = 600;
	const UINT window_starting_width_ = 800;
	const UINT window_starting_x_ = 100;
	const UINT window_starting_y_ = 200;
private:
	UINT window_height_ = 600;								// current client area height
	UINT window_width_ = 800;								// current client area width
	UINT xPos_ = 100;										// window top left x
	UINT yPos_ = 200;										// window top left y
	UINT main_to_client_offset_x, main_to_client_offset_y;

	RECT main_rect, draw_rect;

public:
	static const bool print_debug_ = true;

	Renderer& GetRenderer();

	bool GetRunningState();
	void SetRunningState(int p_run_state);
	
	void SetWindowHeight(UINT height);
	UINT GetWindowHeight();
	void SetWindowWidth(UINT width);
	UINT GetWindowWidth();
	void SetWindowOffsetX(UINT offset);
	UINT GetWindowOffsetX();
	void SetWindowOffsetY(UINT offset);
	UINT GetWindowOffsetY();
	void SetMTCOffsetX(UINT offset_x);
	UINT GetMTCOffsetX();
	void SetMTCOffsetY(UINT offset_y);
	UINT GetMTCOffsetY();
	
	RECT& GetMainWindowRect();
	void SetMainWindowRect(RECT& rect);
	RECT& GetDrawRect();
	void SetDrawRect(RECT& rect);
};

enum {
	STOPPED,
	RUNNING,
};

#endif