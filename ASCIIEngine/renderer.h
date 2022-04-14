#ifndef ASCIIENGINE_RENDERER_H_
#define ASCIIENGINE_RENDERER_H_

#include <Windows.h>
#include <vector>
#include "geometry.h"

class Renderer
{
public:
	bool instanced = false;

	enum panel {
		TOP_DOWN,
		FIRST_PERSON,
		BACKGROUND,

		NUM_PANELS,
	};

	struct DrawArea {
		// Indexed from left-bottom
		uint32_t xPos, yPos;
		uint32_t width, height;
		AABB aabb[NUM_PANELS];
		void* data;
		bool update;
		int focus, lock_focus;

		BITMAPINFO bmi;

		DrawArea() { xPos = 0; yPos = 0; width = 0; height = 0; data = nullptr; focus = -1; lock_focus = -1; };
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
	
	DrawArea draw_area_;

	//const uint32_t colours[3][2] = { { 0x0, 0x111111}, { 0x0, 0x111133 }, { 0x444444, 0x444444 } };
	const uint32_t td_colour_passive = 0x0, fp_colour_passive = 0x0, bg_colour_passive = 0x444444;
	const uint32_t td_colour_active = 0x111111, fp_colour_active = 0x111133, bg_colour_active = 0x444444;

	Renderer(Rect* draw_rect, uint8_t border_width_);
	void SetDrawArea(Rect* rect, uint8_t border_width);
	void UpdateRenderArea(int panel, Point p_p, uint32_t colour = 0xFF0000, bool valid = false);
	void UpdateRenderArea(int panel, Line p_l, uint32_t colour = 0x666666, bool valid = false);
	void UpdateRenderArea(int panel, Rect p_r, uint32_t colour = 0x333333, bool valid = false);
	void DrawRenderArea(HDC hdc);
	void ClearRenderArea(bool force = false, int panel = -1, uint32_t colour = UINT32_MAX);
	int Collision(Point p, int panel = -1);
	int GetFocus();
	void SetFocus(int panel);
	int GetFocusLock();
	void SetFocusLock(int panel);
	void CleanUp();

	UINT* GetMemoryLocation(int panel, Point p);

private:
	Point Clamp(Point &p, Point max);
	bool Validate(int panel, Point p);
	bool Validate(int panel, Line l);
	bool Validate(int panel, Rect rect);
	void DrawLine(Point p1, Point p2, int weight, uint32_t colour);
	Line ClipLine(Line l);
};

#endif