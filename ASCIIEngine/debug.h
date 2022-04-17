#ifndef ASCIIENGINE_DEBUG_H_
#define ASCIIENGINE_DEBUG_H_

#include <Windows.h>
#include <string>
#include "geometry.h"
#include <iomanip>
#include <sstream>

enum calling_class {
	MAIN_WINDOW,
	RENDERER,
	GEOMETRY,

	CLASS_SIZE,
};

enum debug_type {
	MOUSE_POSITION,
	INPUT_DETECTED,
	PANEL_LOCK,

	DEBUG_SIZE,
};

namespace debug {

	void PrintDebugMsg(int calling_class, int debug_type, MSG* msg, int panel_id = -1, int locked_panel = -1);
	std::string ToString(POINT p);

	void ToggleDebugPrinting();
}

#endif