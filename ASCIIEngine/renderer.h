#ifndef ASCIIENGINE_RENDERER_H_
#define ASCIIENGINE_RENDERER_H_

#include <Windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "geometry.h"
#include "debug.h"

class Renderer {
public:
	bool instanced = false;

	enum Panel {
		TOP_DOWN,
		FIRST_PERSON,
		BACKGROUND,

		NUM_PANELS,
	};

	struct DrawArea {
		uint32_t xPos, yPos;
		uint32_t width, height;
		Rect panels[NUM_PANELS];
		void* data;
		bool update;
		int focus, lock_focus;

		BITMAPINFO bmi;

		DrawArea() { xPos = 0; yPos = 0; width = 0; height = 0; data = nullptr; focus = -1; lock_focus = -1; update = false; };
		DrawArea(uint32_t p_xPos, uint32_t p_yPos, uint32_t p_width, uint32_t p_height) {
			xPos = p_xPos;
			yPos = p_yPos;
			width = p_width;
			height = p_height;

			uint32_t data_size = (width * height) * sizeof(uint32_t);
			data = malloc(data_size);

			update = true;
			focus = -1;
			lock_focus = -1;
			
			bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
			bmi.bmiHeader.biWidth = width;
			bmi.bmiHeader.biHeight = height;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
		}

		DrawArea& operator = (DrawArea const& obj) {
			xPos = obj.xPos;
			yPos = obj.yPos;
			width = obj.width;
			height = obj.height;
			data = obj.data;
			bmi = obj.bmi;
			return *this;
		}
	};

	const uint32_t colours[3][2] = { { 0x0, 0x111111}, { 0x0, 0x111133 }, { 0x444444, 0x444444 } };

	Renderer(Rect* draw_rect, uint8_t border_width_);
	DrawArea* getDrawArea();
	Rect getDrawArea(int panelId);
	void setDrawArea(Rect* rect, uint8_t border_width);
	void updateRenderArea(const Point2d& p, int panel, uint32_t colour = 0xFF0000, bool valid = false);
	void updateRenderArea(const Camera& c, int panel, uint32_t colour = 0xFFFFFF, bool valid = false);
	void updateRenderArea(Geometry* g, int panel, uint32_t colour = 0xFFFFFF, bool valid = false);
	void updateRenderArea(const Line& l, int panel, uint32_t colour = 0x777777, bool valid = false);
	void renderLineLow(const Point2d& p0, const Point2d& p1, int panel, uint32_t colour = 0x666666, bool valid = false);
	void renderLineHigh(const Point2d& p0, const Point2d& p1, int panel, uint32_t colour = 0x666666, bool valid = false);
	void updateRenderArea(const Tri& t, int panel, uint32_t colour = 0x555555, bool valid = false);
	void updateRenderArea(const Rect& r, int panel, uint32_t colour = 0x333333, bool valid = false);
	void updateRenderArea(const Circle& c, int panel, uint32_t colour = 0xAAAAAA, bool valid = false);
	void updateGeometry();
	void drawRenderArea(HDC hdc);
	void clearRenderArea(bool force = false, int panel = -1, uint32_t colour = UINT32_MAX);
	int getFocus();
	void setFocus(int panel);
	int getFocusLock();
	void setFocusLock(int panel);
	void cleanUp();

	void* getMemoryLocation(int panel, Point2d p);

private:
	DrawArea draw_area_;

	bool validate(const Point2d& p, int panel = -1);
	bool validate(const Camera& c, int panel = -1);
	bool validate(const Line& l, int panel = -1);
	bool validate(const Rect& rect, int panel = -1);
	bool validate(const Circle& c, int panel = -1);
	Line clipLine(Line l);
};

#endif