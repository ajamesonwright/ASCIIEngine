#ifndef ASCIIENGINE_RENDERER_H_
#define ASCIIENGINE_RENDERER_H_

#include <thread>
#include <Windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "geometry.h"
#include "debug.h"
#include <thread>

class Renderer {
public:
	bool instanced = false;
	static const int NUM_BUFFERS = 2;

	enum Panel {
		TOP_DOWN,
		FIRST_PERSON,
		BACKGROUND,

		NUM_PANELS,
	};

	enum Dimension {
		HORIZONTAL,
		VERTICAL
	};

	enum ClipType {
		LEFT = 0b1,
		RIGHT = 0b10,
		TOP = 0b100,
		BOTTOM = 0b1000
	};

	struct DrawArea {
		uint32_t xPos, yPos;
		uint32_t width, height;
		uint16_t firstPersonHeight = 20;
		uint16_t screensWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		uint16_t screensHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		Rect panels[NUM_PANELS];
		void* data/*[NUM_BUFFERS]*/;
		bool update;
		int focus, lockFocus;

		BITMAPINFO bmi;

		DrawArea();
		DrawArea(uint32_t p_xPos, uint32_t p_yPos, uint32_t p_width, uint32_t p_height);

		DrawArea& operator = (DrawArea const& obj) {
			xPos = obj.xPos;
			yPos = obj.yPos;
			width = obj.width;
			height = obj.height;
			/*for (int i = 0; i < NUM_BUFFERS; i++) {
				data[i] = obj.data[i];
			}*/
			data = obj.data;
			bmi = obj.bmi;
			return *this;
		}
	};

	const std::string BRIGHTNESS_SCALE = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";
	const uint32_t colours[3][2] = { { 0x0, 0x111111}, { 0x0, 0x111133 }, { 0x444444, 0x444444 } };

	static const int PANEL_COUNT = 2;
	static const int THREAD_COUNT = 1;
	std::thread threadPool[PANEL_COUNT][THREAD_COUNT];
	//std::thread bacThread = std::thread() // bac = background and clear thread

	Renderer(Rect* drawRect, uint8_t borderWidth);
	void init(const Camera& camera);
	DrawArea* getDrawArea();
	Rect getDrawArea(int panelId);
	void setDrawArea(Rect* rect, uint8_t borderWidth);
	void clampDimension(uint32_t& dim, Dimension d, int panel = -1);
	uint16_t validate(Geometry* g, uint32_t bounds[], int panel = -1);
	Line clipLine(const Line& l, const uint32_t bounds[], const uint16_t& clipType, int panel);
	Rect clipRect(const Rect& r, const uint32_t bounds[], const uint16_t& clipType, int panel);
	void updateRenderArea(const std::vector<Geometry*>& geometry, const Camera& camera);
	void updateRenderArea(std::vector<uint8_t> character, uint32_t x, uint32_t y, const int& bufferId);
	void updateRenderArea(const Point2d& p, int panel, uint32_t colour = 0xFF0000, bool valid = false);
	void updateRenderArea(const Camera& c, int panel, uint32_t colour = 0xFFFFFF, bool valid = false);
	void updateRenderArea(Geometry* g, int panel, uint32_t colour = 0xFFFFFF, bool valid = false);
	void updateRenderArea(const Line& l, int panel, uint32_t colour = 0x777777, bool valid = false);
	void renderLineLow(const Point2d& p0, const Point2d& p1, int panel, uint32_t colour = 0x666666, bool valid = false);
	void renderLineHigh(const Point2d& p0, const Point2d& p1, int panel, uint32_t colour = 0x666666, bool valid = false);
	void updateRenderArea(const Tri& t, int panel, uint32_t colour = 0x555555, bool valid = false);
	void updateRenderArea(const Rect& r, int panel, uint32_t colour = 0x333333, bool valid = false);
	void updateRenderArea(const Circle& c, int panel, uint32_t colour = 0xAAAAAA, bool valid = false);
	void drawRenderArea(HDC hdc);
	static void clearRenderArea(Renderer* renderer, const bool& force = false, const int& panel = -1, const uint32_t& colour = UINT32_MAX);
	static void updateRenderArea(Renderer* renderer, const Camera& camera, const int& bufferId);
	int getFocus();
	void setFocus(int panel);
	int getFocusLock();
	void setFocusLock(int panel);
	void cleanUp();

	void* getMemoryLocation(int panel, Point2d p);

	std::vector<uint8_t> getCharacterBitmap(float brightness);
	uint8_t getCharacterTileWidth();
	uint8_t getCharacterTileHeight();

	void (*clearRenderAreaFunc)(Renderer* instance, const bool& force, const int& panel, const uint32_t& colour) = &Renderer::clearRenderArea;
	void (*updateRenderAreaFunc)(Renderer* instance, const Camera& camera, const int& bufferId) = &Renderer::updateRenderArea;

private:
	DrawArea drawArea;

	std::vector<uint8_t> getCharacterBitmap(char c);

	uint16_t validate(const Point2d& p, uint32_t bounds[], int panel = -1);
	uint16_t validate(const Line& l, uint32_t bounds[], int panel = -1);
	uint16_t validate(const Rect& rect, uint32_t bounds[], int panel = -1);
	uint16_t validate(const Circle& c, uint32_t bounds[], int panel = -1);
	uint16_t validate(const Camera& c, uint32_t bounds[], int panel = -1);
	void clampDimension(uint32_t& dim, const uint32_t lower, const uint32_t upper, const uint32_t screenDim);
};

#endif