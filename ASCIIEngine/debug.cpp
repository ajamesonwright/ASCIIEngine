#include "debug.h"
#include "geometry.h"
#include "input.h"

// current flag used to determine which debug messaging types are active
/*  Print flag decoding
	0
	b
	0 - camera status
	0 - input_status
	0 - frames per second
	0 - draw mode changed
	'
	0 - geo queue mod
	0 - panel lock
	0 - input detected
	0 - mouse position
*/
int print = 0b0000'0000;
// storage for print flag to allow toggling
int stored_print_flag = 0b0000'0000;

std::string calling_classes[calling_class::CLASS_SIZE] = { "main_window", "renderer", "geometry", "input" };
std::string debug_types[debug_type::DEBUG_SIZE] = { "mouse_position", "input_detected", "panel_lock", "geo_queue_mod", "draw_mode_changed", "frames_per_second", "input_status", "camera_status" };
#define TAB ":\t"
#define DTAB ":\t\t"

namespace {
	void Print(std::string mp) {
		std::wstring stemp = std::wstring(mp.begin(), mp.end());
		LPCWSTR sw = stemp.c_str();
		OutputDebugString(sw);
	};
	bool PrintCode(int debug_type) {
		int code = 1 << debug_type;
		if ((print & code) >> debug_type == 1)
			return true;
		return false;
	};
	std::string ToString(POINT p) {
		std::string point_to_string = "( " + std::to_string(p.x) + ", " + std::to_string(p.y) + " )";
		return point_to_string;
	};
	std::string TypeString(int type) {
		switch (type) {
		case 0:
		{
			return "LINE";
		} break;
		case 1:
		{
			return "TRI";
		} break;
		case 2:
		{
			return "RECT";
		} break;
		case 3:
		{
			return "QUAD";
		} break;
		case 4:
		{
			return "CIRCLE";
		} break;
		default:
			return "UNKNOWN GEOMETRY TYPE";
		}
	};
}

void debug::PrintDebugMsg(int calling_class, int debug_type, MSG* msg, int panel_id, int locked_panel, Geometry* obj, int draw_mode, float fps, Input* input, Ray2d* camera) {

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
	if (PrintCode(debug_type)) {
		switch (debug_type) {
		case MOUSE_POSITION:
		{
			mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + ToString(msg->pt) + TAB + "panel: " + std::to_string(panel_id) + "\n";
		} break;
		case INPUT_DETECTED:
		{
			mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + std::to_string(msg->message) + ":" + std::to_string(msg->wParam) + "\n";
		} break;
		case PANEL_LOCK:
		{
			if (locked_panel == -1 && panel_id != -1)
				mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + " LOCKED \t ID: " + std::to_string(panel_id) + "\n";
			else
				mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + " UNLOCKED \t ID: " + std::to_string(locked_panel) + "\n";
		} break;
		case GEO_QUEUE_MOD:
		{
			if (obj) {
				if (locked_panel)
					mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + TypeString(obj->type) + " added to queue\n";
				else
					mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + TypeString(obj->type) + " removed from queue\n";
			} else
			mp += "Error: object not found.\n";
		} break;
		case DRAW_MODE_CHANGED:
		{
			mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + " draw mode changed to " + TypeString(draw_mode) + "\n";
		} break;
		case FRAMES_PER_SECOND:
		{
			mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + " FPS = " + std::to_string(1.0f / fps) + "\n";
		} break;
		case INPUT_STATUS:
		{
			bool return_early = true;
			mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + " Input = ";
			for (int i = 1; i < KEY_SIZE; i++) {
				if (input->GetInput(i))
					return_early = false;
				mp += std::to_string(input->GetInput(i)) + " ";
			}
			mp += "\n";
			
			if (return_early)
				return;
		} break;
		case CAMERA_STATUS:
		{
			mp += calling_classes[calling_class] + DTAB + debug_types[debug_type] + TAB + " Camera: \tx:" + std::to_string(camera->x) + "\t\t\ty: " + std::to_string(camera->y) + "\t\t\tdir: " + std::to_string(camera->direction) + "\n";
			mp += "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tpx: " + std::to_string(camera->px) + "\tpy: " + std::to_string(camera->py) + "\n";
			mp += "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tvx: " + std::to_string(camera->vx) + "\tvy: " + std::to_string(camera->vy) + "\n";
			mp += "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tax: " + std::to_string(camera->ax) + "\tay: " + std::to_string(camera->ay) + "\n";
		} break;
		}
		Print(mp);
	}
}

void debug::ToggleDebugPrinting() {
	int temp = print;
	print = stored_print_flag;
	stored_print_flag = temp;
}