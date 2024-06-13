#include "debug.h"
#include "geometry.h"
#include "input.h"

// current flag used to determine which debug messaging types are active
/*  Print flag decoding
	0
	b
	0
	0 - quadtree toString
	0 - quadrant dims
	0 - quadrant not found
	'
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
int print = 0b0100'0000'1000;
// storage for print flag to allow toggling
int stored_print_flag = 0b0000'0000'0000;

std::string plainTextCallingClasses[CallingClasses::CLASS_SIZE] = { "main_window", "renderer", "geometry", "input", "quadtree"};
std::string plainTextDebugTypes[DebugTypes::DEBUG_SIZE] = { "mouse_position", "input_detected", "panel_lock", "geo_queue_mod", "draw_mode_changed", "frames_per_second", "input_status", "camera_status", "quadrant_not_found", "quadrant_dims", "quadtree_tostring"};
#define TAB ":\t"
#define DTAB ":\t\t"

// Anonymous namespace to hide internal helper functions
namespace {
	void PrintMessage(std::string mp) {
		std::wstring stemp = std::wstring(mp.begin(), mp.end());
		LPCWSTR sw = stemp.c_str();
		OutputDebugString(sw);
	};
	bool PrintCode(int debugType) {
		int code = 1 << debugType;
		if ((print & code) >> debugType == 1)
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

//void debug::PrintDebugMsg(int *debugMsg.GetCallingClass(), int *debugMsg.GetDebugType(), MSG* msg, int panel_id, int *debugMsg.GetLockedPanel(), Geometry* obj, int draw_mode, float fps, Input* input, Camera* camera) {
void Debug::Print(Debug::DebugMessage* debugMsg) {

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

	std::string mp = "";

	MSG* msg = debugMsg->getMsg();
	if (msg) {
		// Establish timestamp to attach to each debug message (HH:MM:SS.MS):
		int hours = (int)(msg->time / (1000 * 60 * 60));
		int minutes = (int)((msg->time - (hours * 1000 * 60 * 60)) / (1000 * 60));
		float seconds = ((msg->time - (hours * 1000 * 60 * 60) - (minutes * 1000 * 60)) / (1000.f));
		std::stringstream s;
		std::string zero = "0";

		s << "[" << hours << ":" << (minutes < 10 ? zero : "") << minutes << ":" << std::fixed << std::setprecision(3) << (seconds < 10 ? zero : "") << seconds << "] -> ";;

		mp = s.str();
	}

	if (PrintCode(debugMsg->getDebugType())) {
		int caller = debugMsg->getCallingClass();
		int type = debugMsg->getDebugType();
		switch (type) {
		case MOUSE_POSITION:
		{
			mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + ToString(msg->pt) + TAB + "panel: " + std::to_string(debugMsg->getPanelId()) + "\n";
		} break;
		case INPUT_DETECTED:
		{
			mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + std::to_string(msg->message) + ":" + std::to_string(msg->wParam) + "\n";
		} break;
		case PANEL_LOCK:
		{
			if (debugMsg->getLockedPanel() == -1 && debugMsg->getPanelId() != -1)
				mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + " LOCKED \t ID: " + std::to_string(debugMsg->getPanelId()) + "\n";
			else
				mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + " UNLOCKED \t ID: " + std::to_string(debugMsg->getLockedPanel()) + "\n";
		} break;
		case GEO_QUEUE_MOD:
		{
			Geometry* obj = debugMsg->getGeometry();
			if (obj) {
				if (debugMsg->getDrawMode() == 1)
					mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + TypeString(obj->type) + " added to queue\n";
				else
					mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + TypeString(obj->type) + " removed from queue\n";
			} else
				mp += "Error: object not found.\n";
		} break;
		case DRAW_MODE_CHANGED:
		{
			mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + " draw mode changed to " + TypeString(debugMsg->getDrawMode()) + "\n";
		} break;
		case FRAMES_PER_SECOND:
		{
			mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + " FPS = " + std::to_string(1.0f / debugMsg->getFps()) + "\n";
		} break;
		case INPUT_STATUS:
		{
			bool return_early = true;
			mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + " Input = ";
			for (int i = 1; i < KEY_SIZE; i++) {
				Input* input = debugMsg->getInput();
				if (input->getInput(i))
					return_early = false;
				mp += std::to_string(input->getInput(i)) + " ";
			}
			mp += "\n";

			if (return_early)
				return;
		} break;
		case CAMERA_STATUS:
		{
			Camera* camera = debugMsg->getCamera();
			mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + " Camera: \tx:" + std::to_string(camera->x) + "\t\t\ty: " + std::to_string(camera->y) + "\t\t\tdir: " + std::to_string(camera->direction) + "\n";
			mp += "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tpx: " + std::to_string(camera->px) + "\tpy: " + std::to_string(camera->py) + "\n";
			mp += "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tvx: " + std::to_string(camera->vx) + "\tvy: " + std::to_string(camera->vy) + "\n";
			mp += "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tax: " + std::to_string(camera->ax) + "\tay: " + std::to_string(camera->ay) + "\n";
		} break;
		case QUADRANT_NOT_FOUND:
		{
			mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + TAB + "\n";
		} break;
		case QUADRANT_DIMS:
		{
			mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + " Quadrant dimensions: " + debugMsg->getOutputString();
		} break;
		case QUADTREE_TOSTRING:
		{
			mp += plainTextCallingClasses[caller] + DTAB + plainTextDebugTypes[type] + " Quadtree:\n" + debugMsg->getOutputString();
		}
		}
		PrintMessage(mp);
	}
}

void Debug::ToggleDebugPrinting() {
	int temp = print;
	print = stored_print_flag;
	stored_print_flag = temp;
}

Debug::DebugMessage::~DebugMessage() {

}

void Debug::DebugMessage::Print() {
	Debug::Print(this);
}

void Debug::DebugMessage::setOutputString(std::string str) {

	const char* inputString = str.c_str();
	strcpy_s(outputString, strlen(inputString) * sizeof(char) + 1, inputString);
}

int Debug::DebugMessage::getCallingClass() {
	if (this->callingClass > -1) {
		return callingClass;
	}
	return NULL;
}

int Debug::DebugMessage::getDebugType() {
	if (this->debugType > -1) {
		return debugType;
	}
	return NULL;
}

int Debug::DebugMessage::getPanelId() {
	if (this->panelId > -1) {
		return panelId;
	}
	return NULL;
}

int Debug::DebugMessage::getLockedPanel() {
	if (this->lockedPanel > -1) {
		return lockedPanel;
	}
	return NULL;
}

int Debug::DebugMessage::getDrawMode() {
	if (this->drawMode > -1) {
		return drawMode;
	}
	return NULL;
}

float Debug::DebugMessage::getFps() {
	if (this->fps > 0) {
		return fps;
	}
	return NULL;
}

MSG* Debug::DebugMessage::getMsg() {
	if (msg) {
		return msg;
	}
	return nullptr;
}

Geometry* Debug::DebugMessage::getGeometry() {
	if (obj) {
		return obj;
	}
	return nullptr;
}

Input* Debug::DebugMessage::getInput() {
	if (input) {
		return input;
	}
	return nullptr;
}

Camera* Debug::DebugMessage::getCamera() {
	if (camera) {
		return camera;
	}
	return nullptr;
}

std::string Debug::DebugMessage::getOutputString() {
	return outputString;
}