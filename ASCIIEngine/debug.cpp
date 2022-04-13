#include "debug.h"

std::string calling_classes[calling_class::CLASS_SIZE] = { "main_window", "renderer", "geometry" };
std::string debug_types[debug_type::DEBUG_SIZE] = { "mouse_position", "mouse_memory_location", "panel_id", "input_detected", "panel_lock" };

void debug::PrintDebug(int calling_class, int debug_type, Point& p, int counter, UINT* memory_location, int panel_id, int input, int locked_panel) {
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
	if (debug_type == INPUT_DETECTED) {
		mp = calling_classes[calling_class] + ":\t" + debug_types[debug_type] + " -> " + std::to_string(input) + "\n";
	}
	if (debug_type == PANEL_LOCK) {
		if (locked_panel == -1 && panel_id != -1)
			mp = calling_classes[calling_class] + ": " + debug_types[debug_type] + " -> : panel LOCKED \t ID: " + std::to_string(panel_id) + "\n";
		else
			mp = calling_classes[calling_class] + ": " + debug_types[debug_type] + " -> : panel UNLOCKED \t ID: " + std::to_string(locked_panel) + "\n";
	}
	std::wstring stemp = std::wstring(mp.begin(), mp.end());
	LPCWSTR sw = stemp.c_str();
	OutputDebugString(sw);
}