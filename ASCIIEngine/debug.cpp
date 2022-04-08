#include "debug.h"

std::string calling_classes[calling_class::CLASS_SIZE] = { "main_window", "renderer", "geometry" };
std::string debug_types[debug_type::DEBUG_SIZE] = { "mouse_position", "mouse_memory_location", "panel_id" };

void debug::PrintDebug(int calling_class, int debug_type, Point& p, int counter, UINT* memory_location, int panel_id) {
	std::string mp;
	if (debug_type == MOUSE_POSITION) {

		mp = calling_classes[calling_class] + ":\t" + std::to_string(counter) + "\t " + debug_types[debug_type] + ": (" + std::to_string(p.x) + ", " + std::to_string(p.y) + ") \n";
	}
	if (debug_type == MOUSE_MEMORY_LOCATION) {
		if (memory_location)
			mp = calling_classes[calling_class] + ":\t\t " + debug_types[debug_type] + ": (" + std::to_string(*memory_location) + ") \n";
		else
			mp = "Error detecting memory location of cursor.";
	}
	if (debug_type == PANEL_ID) {
		mp = calling_classes[calling_class] + ":\t\t\t" + debug_types[debug_type] + ": " + std::to_string(panel_id) + ") \n";
	}
	std::wstring stemp = std::wstring(mp.begin(), mp.end());
	LPCWSTR sw = stemp.c_str();
	OutputDebugString(sw);
}