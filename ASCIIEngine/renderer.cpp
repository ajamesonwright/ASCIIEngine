#include "renderer.h"

Renderer::Renderer(Rect* draw_rect, uint8_t border_width) {

	if (instanced)
		return;
	instanced = true;

	draw_area_[TOP_DOWN] = DrawArea(draw_rect->left + border_width, draw_rect->bottom - border_width, (draw_rect->left + draw_rect->right) / 2 - (2 * border_width), draw_rect->bottom - 2 * border_width);
	draw_area_[FIRST_PERSON] = DrawArea((draw_rect->left + draw_rect->right) / 2 + border_width, draw_rect->bottom - border_width, (draw_rect->left + draw_rect->right) / 2 - (2 * border_width), draw_rect->bottom - 2 * border_width);
	draw_area_[BACKGROUND] = DrawArea(draw_rect->left, draw_rect->bottom, draw_rect->right - draw_rect->left, draw_rect->bottom - draw_rect->top);
}

void Renderer::SetDrawArea(int panel, Rect* rect_inc, uint8_t border_width) {

	/*
	* Each draw_area_ object is configured for a specific range.
	* draw_area_[BACKGROUND] is the entire range of the client rect { 1604, 561 } zero indexed.
						(FUTURE OPTIMIZATION IS TO INCLUDE GUARDS IN THE UPDATE CALL FOR THIS PANEL TO LIMIT
						 UPDATED PIXELS ONLY TO THOSE THAT ARE NOT ON TOP OF THE OTHER PANELS)
	* draw_area_[TOP_DOWN] is the range from:		LT = (dr.left + BW, dr.top + BW)						BR = ((dr.left + dr.right) / 2 - BW, dr.bottom - BW)
	* draw_area_[FIRST_PERSON] is the range from	LT = ((dr.left + dr.right) / 2 + BW, dr.bottom - BW)	BR = (dr.right - BW, dr.bottom - BW)
	* Using WindowRect dimensions of 1620 by 800, yields total area of 1604W x 561H, zero indexed.
	*/
	Rect rect = *rect_inc;

	switch (panel) {
	case TOP_DOWN:
	{
		draw_area_[panel].xPos = rect.left + border_width;
		draw_area_[panel].yPos = rect.bottom - border_width;
		draw_area_[panel].width = (rect.right - rect.left) / 2 - 2 * border_width;
		draw_area_[panel].height = (rect.bottom - rect.top) - 2 * border_width;
	} break;
	case FIRST_PERSON:
	{
		draw_area_[panel].xPos = (rect.left + rect.right) / 2 + border_width;
		draw_area_[panel].yPos = rect.bottom - border_width;
		draw_area_[panel].width = (rect.right - rect.left) / 2 - 2 * border_width;
		draw_area_[panel].height = (rect.bottom - rect.top) - 2 * border_width;
	} break;
	case BACKGROUND:
	{
		draw_area_[panel].xPos = rect.left;
		draw_area_[panel].yPos = rect.bottom;
		draw_area_[panel].width = rect.right - rect.left;
		draw_area_[panel].height = rect.bottom - rect.top;
	} break;
	}

	uint32_t data_size = (draw_area_[panel].width * draw_area_[panel].height) * sizeof(uint32_t);

	draw_area_[panel].update = true;

	// Clear on resize when already full
	if (draw_area_[panel].data)
		free(draw_area_[panel].data);
	draw_area_[panel].data = malloc(data_size);

	draw_area_[panel].bmi.bmiHeader.biSize = sizeof(draw_area_[panel].bmi.bmiHeader);
	draw_area_[panel].bmi.bmiHeader.biWidth = draw_area_[panel].width;
	draw_area_[panel].bmi.bmiHeader.biHeight = draw_area_[panel].height;
	draw_area_[panel].bmi.bmiHeader.biPlanes = 1;
	draw_area_[panel].bmi.bmiHeader.biBitCount = 32;
	draw_area_[panel].bmi.bmiHeader.biCompression = BI_RGB;
}

void Renderer::UpdateRenderArea(int panel, Point p_p, uint32_t colour, bool valid) {

	Point p = p_p;
	if (!valid)
		if (!Validate(panel, p))
			return;

	// Draws from bottom left to top right as first memory location indicates bottom left corner
	uint32_t* pixel = (uint32_t*)draw_area_[panel].data + (draw_area_[panel].width * draw_area_[panel].height) - (p.y * draw_area_[panel].width) + p.x;
		*pixel = colour;
	
	draw_area_[panel].update = true;
}

void Renderer::UpdateRenderArea(int panel, Line p_l, uint32_t colour, bool valid) {
	
	if (!valid)
		if (!Validate(panel, p_l))
			// TODO: implement clip line instead of simple return
			return;

	// for now, assuming that no rasterization is required, line is horizontal, and a is to the left of b
	Point p = p_l.a;
	for (int i = 0; i < (p_l.b.x - p_l.a.x); i++) {
		// note: valid flag will need to account for clipping (maybe alter line object sent for rendering to ensure all points reside in viewport?)
		UpdateRenderArea(panel, p, colour, true);
		p.x++;
	}

	draw_area_[panel].update = true;
}

void Renderer::UpdateRenderArea(int panel, Rect p_rect, uint32_t colour, bool valid) {
	// Used for drawing UI elements, since they will typically be the only objects represented as rectangles
	if (!valid)
		if (!Validate(panel, p_rect))
			// TODO: implement clip line instead of simple return
			return;

	// for now, assuming that no rasterization is required, all lines are vertical or horizontal
	Rect rect = p_rect;
	// not optimized for fewest iterations (ie. does not determine whether it is more efficient to do length then width or vice versa)
	Point p = { rect.left, rect.bottom };
	for (int i = rect.bottom; i > rect.top; i--) {
		for (int j = rect.left; j <= rect.right; j++) {
			UpdateRenderArea(panel, p, colour, true);
			p.x++;
		}
		p.x = rect.left;
		p.y--;
	}

	draw_area_[panel].update = true;
}

void Renderer::DrawRenderArea(int panel, HDC hdc) {

	if (!draw_area_[panel].update)
		return;

	// Draws selected DrawArea from top left to bottom right
	StretchDIBits(hdc, draw_area_[panel].xPos, draw_area_[panel].yPos - draw_area_[panel].height, draw_area_[panel].width, draw_area_[panel].height, 0, 0, draw_area_[panel].width, draw_area_[panel].height, draw_area_[panel].data, &draw_area_[panel].bmi, DIB_RGB_COLORS, SRCCOPY);
	draw_area_[panel].update = false;
}

void Renderer::CleanUp() {

	if (!this)
		return;
	for (int i = 0; i < NUM_PANELS; i++) {
		if (draw_area_[i].data)
			free(draw_area_[i].data);
	}
}

void Renderer::ClearRenderArea(int panel, uint32_t colour, bool force) {

	if (!force) {
		if (!draw_area_[panel].update)
			return;
	}

	DrawArea da;
	AABB draw_areas[2];
	if (panel == 2) {
		// Create a collision area to detect when the current pixel is inside TOP_DOWN or FIRST_PERSON
		for (int i = 0; i < NUM_PANELS - 1; i++) {
			DrawArea da = draw_area_[i];
			draw_areas[i] = AABB(Rect(da.xPos, da.yPos - da.height, da.xPos + da.width, da.yPos));
		}
	}
	// Starting from first element in data struct (corresponds to bottom left pixel), iterate and set all bits to same value
	uint32_t* pixel = (uint32_t*)draw_area_[panel].data;
	for (int i = 0; i < draw_area_[panel].height; i++)
	{
		for (int j = 0; j < draw_area_[panel].width; j++)
		{
			if (panel == 2) {
				// check for collision between BACKGROUND AND TOP_DOWN or FIRST_PERSON and skip rendering of pixel
				Point current_pixel = Point(j, i);
				if (draw_areas[0].Collision(current_pixel) || draw_areas[1].Collision(current_pixel)) {
					*pixel++;
					continue;
				}
			}
			*pixel++ = colour;
		}
	}

	draw_area_[panel].update = true;
}

Point Renderer::Clamp(Point &p, Point max) {

	if (p.x >= max.x)		p.x = max.x;
	if (p.x < 0)			p.x = 0;
	if (p.y >= max.y)		p.y = max.y;
	if (p.y < 0)			p.y = 0;

	return p;
}

uint32_t* Renderer::GetMemoryLocation(int panel, Point p) {

	uint32_t cursorMemoryLocation = (uint32_t)draw_area_[panel].data + p.x + p.y * draw_area_[panel].width;
	return &cursorMemoryLocation;
}

bool Renderer::Validate(int panel, Point p) {

	return (p.x >= 0 &&	p.x < draw_area_[panel].width && p.y >= 0 && p.y < draw_area_[panel].height);
}

bool Renderer::Validate(int panel, Line l) {

	return (Validate(panel, l.a) && Validate(panel, l.b));
}

bool Renderer::Validate(int panel, Rect rect) {

	Line l = Line { Point { (uint16_t)rect.left, (uint16_t)rect.top }, Point { (uint16_t)rect.right, (uint16_t)rect.bottom } };
	return (Validate(panel, l.a) && Validate(panel, l.b));
}

void Renderer::DrawLine(Point p1, Point p2, int weight, uint32_t colour)
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
	
	if (!(Validate(1, p1) && Validate(1, p2)))
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