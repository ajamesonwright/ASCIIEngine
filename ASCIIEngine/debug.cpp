#include "debug.h"
#include "geometry.h"

// print flag to toggle all debug printing
bool print = true;

std::string calling_classes[calling_class::CLASS_SIZE] = { "main_window", "renderer", "geometry" };
std::string debug_types[debug_type::DEBUG_SIZE] = { "mouse_position", "input_detected", "panel_lock", "geo_queue" };
#define TAB ":\t"
#define DTAB ":\t\t"

void debug::PrintDebugMsg(int calling_class, int debug_type, MSG* msg, int panel_id, int locked_panel, Geometry* obj) {

	if (!print)
		return;

	/*
	HWND        hwnd;
	UINT        message;
	WPARAM      wParam;
	LPARAM      lParam;
	DWORD       time; assuming ms
	POINT       pt;
	*/

	// Establish timestamp to attach to each debug message (HH:MM:SS.MS):
	int hours = (int)(msg->time / (1000 * 60 * 60));
	int minutes = (int)((msg->time - (hours * 1000 * 60 * 60)) / (1000 * 60));
	float seconds = ((msg->time - (hours * 1000 * 60 * 60) - (minutes * 1000 * 60)) / (1000.f));
	std::stringstream s;
	std::string zero = "0";

	s << "[" << hours << ":" << (minutes < 10 ? zero : "") << minutes << ":" << std::fixed << std::setprecision(3) << (seconds < 10 ? zero : "") << seconds << "] -> ";;

	std::string mp = s.str();
	if (debug_type == MOUSE_POSITION) {
		mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + ToString(msg->pt) + TAB + "panel: " + std::to_string(panel_id) + "\n";
	}
	if (debug_type == INPUT_DETECTED) {
			mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + std::to_string(msg->message) + "\n";
	}
	if (debug_type == PANEL_LOCK) {
		if (locked_panel == -1 && panel_id != -1)
			mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + " LOCKED \t ID: " + std::to_string(panel_id) + "\n";
		else
			mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + " UNLOCKED \t ID: " + std::to_string(locked_panel) + "\n";
	}
	if (debug_type == GEO_QUEUE_MOD) {
		if (obj) {
			if (locked_panel)
				mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + obj->GeometryTypeString() + " added to queue\n";
			else
				mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + obj->GeometryTypeString() + " removed from queue\n";
		}
		else
			mp += "Error: object not found.\n";
	}

	std::wstring stemp = std::wstring(mp.begin(), mp.end());
	LPCWSTR sw = stemp.c_str();
	OutputDebugString(sw);
}

std::string debug::ToString(POINT p) {
	std::string point_to_string = "( " + std::to_string(p.x) + ", " + std::to_string(p.y) + " )";
	return point_to_string;
}

void debug::ToggleDebugPrinting() {
	print = !print;
}

