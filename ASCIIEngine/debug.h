#ifndef ASCIIENGINE_DEBUG_H_
#define ASCIIENGINE_DEBUG_H_

#include <Windows.h>
#include <string>
#include "geometry.h"

enum calling_class {
	MAIN_WINDOW = 0,
	RENDERER = 1,
	GEOMETRY = 2,
	CLASS_SIZE = 3,
};

enum debug_type {
	MOUSE_POSITION = 0,
	MOUSE_MEMORY_LOCATION = 1,
	PANEL_ID = 2,
	DEBUG_SIZE = 3,
};

namespace debug {

	void PrintDebug(int calling_class, int debug_type, Point& p, int counter = 0, UINT* memory_location = nullptr, int panel_id = -1);
}

#endif