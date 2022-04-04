#ifndef ASCIIENGINE_RENDERER_H_
#define ASCIIENGINE_RENDERER_H_

#include <Windows.h>
#include <vector>
#include "geometry.h"

class Renderer
{
	struct DrawArea {
		int xPos, yPos;
		int width, height;
		void* data = nullptr;

		BITMAPINFO bmi;
	} draw_area_;

public:
	void SetDrawArea(RECT* rect);
	void UpdateRenderArea(POINT p);
	void DrawRenderArea(HDC hdc);
	void CleanUp();
	UINT* GetMemoryLocation(POINT p);
	Renderer(RECT* rect_inc);

private:
	void ClearBuffer(unsigned int colour);
	POINT Clamp(POINT &p, POINT max);
	bool Validate(POINT p);
	void DrawPixel(POINT p, unsigned int colour);
	void DrawLine(POINT p, std::vector<int> vec_3d, int weight, unsigned int colour);
	void DrawLine(POINT p1, POINT p2, int weight, unsigned int colour);
	Line ClipLine(Line l);
};

#endif