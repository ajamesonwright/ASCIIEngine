#ifndef ASCIIENGINE_DEBUG_H_
#define ASCIIENGINE_DEBUG_H_

#include <Windows.h>
#include <string>

enum calling_class {
	MAIN_WINDOW = 0,
	RENDERER = 1,
	GEOMETRY = 2,
	CLASS_SIZE = 3,
};

enum debug_type {
	MOUSE_POSITION = 0,
	MOUSE_MEMORY_LOCATION = 1,
	DEBUG_SIZE = 2,
};

namespace debug {

	void PrintDebug(int calling_class, int debug_type, POINT& p, int counter, UINT* memory_location = nullptr);
}

#endif