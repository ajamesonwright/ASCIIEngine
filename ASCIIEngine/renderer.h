#ifndef ASCIIENGINE_RENDERER_H_
#define ASCIIENGINE_RENDERER_H_

#include <Windows.h>
#include <vector>
#include "geometry.h"

class Renderer
{
	struct DrawArea {
		uint32_t xPos = 0, yPos = 0;
		uint32_t width = 0, height = 0;
		void* data = nullptr;

		BITMAPINFO bmi;
	} draw_area_;

public:
	Renderer(Rect* rect_inc);

	void SetDrawArea(Rect* rect);
	void UpdateRenderArea(Point p_p, unsigned int colour = 0xFF0000);
	void UpdateRenderArea(Line p_l, unsigned int colour = 0x000000);
	void UpdateRenderArea(Rect p_r, unsigned int colour = 0x222222);
	void DrawRenderArea(HDC hdc);
	void ClearRenderArea(unsigned int colour = 0x555555);
	void CleanUp();
	UINT* GetMemoryLocation(POINT p);

private:
	Point Clamp(Point &p, Point max);
	bool Validate(Point p);
	bool Validate(Line l);
	bool Validate(Rect rect);
	void DrawPixel(Point p, unsigned int colour);
	void DrawLine(Point p, std::vector<int> vec_3d, int weight, unsigned int colour);
	void DrawLine(Point p1, Point p2, int weight, unsigned int colour);
	Line ClipLine(Line l);
};

#endif