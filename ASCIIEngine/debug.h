#ifndef ASCIIENGINE_DEBUG_H_
#define ASCIIENGINE_DEBUG_H_

#include <Windows.h>
#include <string>
#include <iomanip>
#include <sstream>

class Geometry;
class Camera;
class Input;

enum CallingClasses {
	MAIN_WINDOW_CLASS,
	RENDERER_CLASS,
	GEOMETRY_CLASS,
	INPUT_CLASS,
	QUADTREE_CLASS,

	CLASS_SIZE,
};

enum DebugTypes {
	MOUSE_POSITION,
	INPUT_DETECTED,
	PANEL_LOCK,
	GEO_QUEUE_MOD,
	DRAW_MODE_CHANGED,
	FRAMES_PER_SECOND,
	INPUT_STATUS,
	CAMERA_STATUS,
	QUADRANT_NOT_FOUND,
	QUADRANT_DIMS,
	QUADTREE_TOSTRING,
	NONE,

	DEBUG_SIZE,
};

namespace Debug {

	class DebugMessage {

	private:
		int callingClass;
		int debugType = NONE;
		int panelId = -1;
		int lockedPanel = -1;
		int drawMode = -1;
		float fps = 0;
		MSG* msg = nullptr;
		Geometry* obj = nullptr;
		Input* input = nullptr;
		Camera* camera = nullptr;
		char outputString[200];

	public:
		DebugMessage(int callingClass) {
			this->callingClass = callingClass;
		}
		DebugMessage(int callingClass, int debugType) {
			this->callingClass = callingClass;
			this->debugType = debugType;
		}
		~DebugMessage();

		void Print();

		// Message setters
		void setDebugType(int debugType) { this->debugType = debugType; }
		void setPanelId(int panelId) { this->panelId = panelId; }
		void setLockedPanel(int lockedPanel) { this->lockedPanel = lockedPanel; }
		void setDrawMode(int drawMode) { this->drawMode = drawMode; }
		void setFps(float fps) { this->fps = 1/fps; }
		void setMsg(MSG* msg) { this->msg = msg; }
		void setGeometry(Geometry* obj) { this->obj = obj; }
		void setInput(Input* input) { this->input = input; }
		void setCamera(Camera* camera) { this->camera = camera; }
		void setOutputString(std::string str);

		// Message getters
		int getCallingClass();
		int getDebugType();
		int getPanelId();
		int getLockedPanel();
		int getDrawMode();
		float getFps();
		MSG* getMsg();
		Geometry* getGeometry();
		Input* getInput();
		Camera* getCamera();
		std::string getOutputString();
	};

	void Print(Debug::DebugMessage* debug_msg);
	//void PrintDebugMsg(int calling_class, int debug_type, MSG* msg, int panel_id = -1, int locked_panel = -1, Geometry* obj = nullptr, int draw_mode = -1, float fps = 0, Input* input = nullptr, Camera* camera = nullptr);
	void ToggleDebugPrinting();
};



#endif