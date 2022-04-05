#include "renderer.h"

Renderer::Renderer(Rect* rect_inc) {
	SetDrawArea(rect_inc);
}

void Renderer::SetDrawArea(Rect* rect_inc) {

	Rect rect = *rect_inc;

	/*
	* draw_area_ is configured to operate in the space between
	* 784 wide and 561 tall, zero indexed.
	* ie. { 0->783, 0->560 } are all valid coordinates
	*/
	draw_area_.xPos = rect.left;
	draw_area_.yPos = rect.top;
	draw_area_.width = rect.right - rect.left;
	draw_area_.height = rect.bottom - rect.top;

	unsigned int data_size = (draw_area_.width * draw_area_.height) * sizeof(unsigned int);

	// Clear on resize when already full
	if (draw_area_.data)
		free(draw_area_.data);
	draw_area_.data = malloc(data_size);

	draw_area_.bmi.bmiHeader.biSize = sizeof(draw_area_.bmi.bmiHeader);
	draw_area_.bmi.bmiHeader.biWidth = draw_area_.width;
	draw_area_.bmi.bmiHeader.biHeight = draw_area_.height;
	draw_area_.bmi.bmiHeader.biPlanes = 1;
	draw_area_.bmi.bmiHeader.biBitCount = 32;
	draw_area_.bmi.bmiHeader.biCompression = BI_RGB;
}

void Renderer::UpdateRenderArea(Point p_p, unsigned int colour) {

	// set pixel pointer to memory location associated with mouse position
	// Draws from bottom to top as first memory location indicates bottom left corner
	Point p;
	p.x = p_p.x;
	p.y = p_p.y;
	if (!Validate(p))
		return;
		//Clamp(p, POINT{ draw_area_.width, draw_area_.height });

	unsigned int* pixel = (unsigned int*)draw_area_.data + (draw_area_.width * draw_area_.height) - (p.y * draw_area_.width) + p.x;
		*pixel = colour;
}

void Renderer::UpdateRenderArea(Line p_l, unsigned int colour) {
	
	if (!Validate(p_l))
		// TODO: implement clip line instead of simple return
		return;

	// for now, assuming that no rasterization is required, line is horizontal, and a is to the left of b
	Point p = p_l.a;
	for (int i = 0; i < (p_l.b.x - p_l.a.x); i++) {
		UpdateRenderArea(p, colour);
		p.x++;
	}
}

void Renderer::UpdateRenderArea(Rect p_rect, unsigned int colour) {
	// Used for drawing UI elements, since they will typically be the only
	// objects represented as quads
	if (!Validate(p_rect))
		// TODO: implement clip line instead of simple return
		return;

	// for now, assuming that no rasterization is required, all lines are vertical or horizontal
	Rect rect = p_rect;
	// not optimized for fewest iterations (ie. does not determine whether it is more efficient to do length then width or vice versa)
	Point p = { rect.left, rect.bottom };
	for (int i = rect.left; i < rect.right; i++) {
		for (int j = rect.top; j < rect.bottom; j++) {
			UpdateRenderArea(p, colour);
			p.x++;
		}
		p.x = rect.left;
		p.y++;
	}
}

void Renderer::DrawRenderArea(HDC hdc) {

	StretchDIBits(hdc, 0, 0, draw_area_.width, draw_area_.height, 0, 0, draw_area_.width, draw_area_.height, draw_area_.data, &draw_area_.bmi, DIB_RGB_COLORS, SRCCOPY);
}

void Renderer::CleanUp() {

	if (!this)
		return;
	if (draw_area_.data)
		free(draw_area_.data);
}

void Renderer::ClearRenderArea(unsigned int colour) {

	// Starting from first element in data struct (corresponds to bottom left pixel), iterate and set all bits to same value
	unsigned int* pixel = (unsigned int*)draw_area_.data;
	for (int i = 0; i < draw_area_.height; i++)
	{
		for (int j = 0; j < draw_area_.width; j++)
		{
			*pixel++ = colour;
		}
	}
}

Point Renderer::Clamp(Point &p, Point max) {

	if (p.x >= max.x)		p.x = max.x;
	if (p.x < 0)			p.x = 0;
	if (p.y >= max.y)		p.y = max.y;
	if (p.y < 0)			p.y = 0;

	return p;
}

UINT* Renderer::GetMemoryLocation(POINT p) {

	unsigned int cursorMemoryLocation = (unsigned int)draw_area_.data + p.x + p.y * draw_area_.width;
	return &cursorMemoryLocation;
}

bool Renderer::Validate(Point p) {

	return (p.x >= 0 &&	p.x <= draw_area_.width && p.y >= 0 && p.y <= draw_area_.height);
}

bool Renderer::Validate(Line l) {

	return (Validate(l.a) && Validate(l.b));
}

bool Renderer::Validate(Rect rect) {

	Line l = Line { Point { (uint16_t)rect.left, (uint16_t)rect.top }, Point { (uint16_t)rect.right, (uint16_t)rect.bottom } };
	return (Validate(l.a) && Validate(l.b));
}

void Renderer::DrawPixel(Point p, unsigned int colour)
{
	
}

void Renderer::DrawLine(Point p, std::vector<int> vec_3d, int weight, unsigned int colour)
{

}

void Renderer::DrawLine(Point p1, Point p2, int weight, unsigned int colour)
{
	/*
		Bresenham algorithm
		Octants defined analogously to Cartesian quadrants
		++, -+, --, +- (except proceeding CW instead of CCW):

			|
		  2	| 3
		---------
		  1	| 0
			|
		0 - (+, +)
		1 - (-, +)
		2 - (-, -)
		3 - (+, -)
	*/
	
	if (!(Validate(p1) && Validate(p2)))
	{

		ClipLine(Line{ p1, p2 });
	}

	

	Point* min_y = nullptr;
	Point* min_x = nullptr;
	Point* max_y = nullptr;
	Point* max_x = nullptr;
	
	
	if (p1.y < p2.y)
	{
		min_y = &p1;
		max_y = &p2;
	}
	if (p1.y > p2.y)
	{
		min_y = &p2;
		max_y = &p1;
	}

	if (p1.x < p2.x)
	{
		min_x = &p1;
		max_x = &p2;
	}
	if (p1.x > p2.x)
	{
		min_x = &p2;
		max_x = &p1;
	}

	// if line is not horizontal, start at lower of the two y values and count over to the higher
	if (min_y)
	{
		for (int i = min_y->y; i < max_y->y; i++)
		{
			if (min_x)
			{
				for (int j = min_x->x; j < max_x->x; j++)
				{

				}
			}
		}
	}
	else
	{

	}
}

Line Renderer::ClipLine(Line l)
{
	return Line{ Point {}, Point {} };
}