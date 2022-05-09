#ifndef ASCIIENGINE_DEBUG_H_
#define ASCIIENGINE_DEBUG_H_

#include <Windows.h>
#include <string>
#include <iomanip>
#include <sstream>

class Geometry;
class Input;

enum calling_class {
	MAIN_WINDOW_CLASS,
	RENDERER_CLASS,
	GEOMETRY_CLASS,
	INPUT_CLASS,

	CLASS_SIZE,
};

enum debug_type {
	MOUSE_POSITION,
	INPUT_DETECTED,
	PANEL_LOCK,
	GEO_QUEUE_MOD,
	DRAW_MODE_CHANGED,
	FRAMES_PER_SECOND,
	INPUT_STATUS,

	DEBUG_SIZE,
};

namespace debug {

	void PrintDebugMsg(int calling_class, int debug_type, MSG* msg, int panel_id = -1, int locked_panel = -1, Geometry* obj = nullptr, int draw_mode = -1, float fps = 0, Input* input = nullptr);
	void ToggleDebugPrinting();
}

#endif