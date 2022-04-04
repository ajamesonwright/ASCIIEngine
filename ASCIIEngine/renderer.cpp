#include "renderer.h"

void Renderer::SetDrawArea(RECT* rect_inc) {

	RECT rect = { rect_inc->left, rect_inc->top, rect_inc->right, rect_inc->bottom };

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
	draw_area_.data = malloc(draw_area_.width * draw_area_.height * sizeof(unsigned int));

	draw_area_.bmi.bmiHeader.biSize = sizeof(draw_area_.bmi.bmiHeader);
	draw_area_.bmi.bmiHeader.biWidth = draw_area_.width;
	draw_area_.bmi.bmiHeader.biHeight = draw_area_.height;
	draw_area_.bmi.bmiHeader.biPlanes = 1;
	draw_area_.bmi.bmiHeader.biBitCount = 32;
	draw_area_.bmi.bmiHeader.biCompression = BI_RGB;
}

void Renderer::UpdateRenderArea(POINT p_inc) {

	unsigned int colour;
	colour = 0xFF0000;
	// set pixel pointer to memory location associated with mouse position
	// Draws from bottom to top as first memory location indicates bottom left corner
	POINT p;
	p.x = p_inc.x;
	p.y = p_inc.y;
	Clamp(p, POINT{ draw_area_.width, draw_area_.height });
	unsigned int* pixel = (unsigned int*)draw_area_.data + (draw_area_.width * draw_area_.height) - (p.y * draw_area_.width) + p.x;
		*pixel = colour;
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

void Renderer::ClearBuffer(unsigned int colour) {

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

POINT Renderer::Clamp(POINT &p, POINT max) {

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

Renderer::Renderer(RECT* rect_inc) {
	RECT rect = { rect_inc->left, rect_inc->top, rect_inc->right, rect_inc->bottom };
	/*
	* draw_area_ is configured to initialize in the space between
	* 784 wide and 561 tall, zero indexed.
	* ie. { 0->783, 0->560 } are all valid coordinates
	*/
	draw_area_.xPos = (int)rect.left;
	draw_area_.yPos = (int)rect.top;
	draw_area_.width = (int)(rect.right - rect.left);
	draw_area_.height = (int)(rect.bottom - rect.top);
	
	int data_size = (draw_area_.width * draw_area_.height * sizeof(int));

	// Clear on resize when already full
	draw_area_.data = malloc(data_size);

	draw_area_.bmi.bmiHeader.biSize = sizeof(draw_area_.bmi.bmiHeader);
	draw_area_.bmi.bmiHeader.biWidth = draw_area_.width;
	draw_area_.bmi.bmiHeader.biHeight = draw_area_.height;
	draw_area_.bmi.bmiHeader.biPlanes = 1;
	draw_area_.bmi.bmiHeader.biBitCount = 32;
	draw_area_.bmi.bmiHeader.biCompression = BI_RGB;
}

bool Renderer::Validate(POINT p)
{
	return (p.x < draw_area_.width && p.y < draw_area_.height);
}

void Renderer::DrawPixel(POINT p, unsigned int colour)
{
	if (!Validate(p))	return;
	
}

void Renderer::DrawLine(POINT p, std::vector<int> vec_3d, int weight, unsigned int colour)
{

}

void Renderer::DrawLine(POINT p1, POINT p2, int weight, unsigned int colour)
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

	

	POINT* min_y = nullptr;
	POINT* min_x = nullptr;
	POINT* max_y = nullptr;
	POINT* max_x = nullptr;
	
	
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
	return Line{ POINT {}, POINT {} };
}