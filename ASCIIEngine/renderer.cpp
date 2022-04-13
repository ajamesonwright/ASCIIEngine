#include "renderer.h"

Renderer::Renderer(Rect* draw_rect, uint8_t border_width) {

	if (instanced)
		return;
	instanced = true;

	draw_area_ = DrawArea();
}

void Renderer::SetDrawArea(Rect* rect_inc, uint8_t border_width) {

	/*
	* A draw_area_ object contains information regarding each panel that will be drawn to screen.
	* draw_area_.aabb[BACKGROUND] is the entire range of the client rect { 1604, 561 } zero indexed.
	* draw_area_.aabb[TOP_DOWN] is the range from:		LT = (dr.left + BW, dr.top + BW)						BR = ((dr.left + dr.right) / 2 - BW, dr.bottom - BW)
	* draw_area_.aabb[FIRST_PERSON] is the range from	LT = ((dr.left + dr.right) / 2 + BW, dr.top + BW)		BR = (dr.right - BW, dr.bottom - BW)
	* Using WindowRect dimensions of 1620 by 800, yields total area of 1604W x 561H, zero indexed.
	*/
	Rect rect = *rect_inc;
	uint32_t data_size = 0;

	draw_area_.xPos = rect.left;
	draw_area_.yPos = rect.bottom;
	draw_area_.width = rect.right - rect.left;
	draw_area_.height = rect.bottom - rect.top;

	data_size = (draw_area_.width * draw_area_.height) * sizeof(uint32_t);

	draw_area_.update = true;

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

	draw_area_.aabb[TOP_DOWN] = AABB(Point(rect.left + border_width, rect.top + border_width), Point((rect.left + rect.right) / 2 - border_width, rect.bottom - border_width));
	draw_area_.aabb[FIRST_PERSON] = AABB(Point((rect.left + rect.right) / 2 + border_width, rect.top + border_width), Point(rect.right - border_width, rect.bottom - border_width));
	draw_area_.aabb[BACKGROUND] = AABB(Point(rect.left, rect.top), Point(rect.right, rect.bottom));
}

void Renderer::UpdateRenderArea(int panel, Point p_p, uint32_t colour, bool valid) {

	Point p = p_p;
	if (!valid)
		if (!Validate(panel, p))
			return;

	// Draws from bottom left to top right as first memory location indicates bottom left corner
	uint32_t* pixel = (uint32_t*)draw_area_.data + (draw_area_.width * draw_area_.height) - (p.y * draw_area_.width) + p.x;
		*pixel = colour;
	
	draw_area_.update = true;
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

	draw_area_.update = true;
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

	draw_area_.update = true;
}

void Renderer::DrawRenderArea(HDC hdc) {

	if (!draw_area_.update)
		return;

	// Draws selected DrawArea from top left to bottom right
	StretchDIBits(hdc, draw_area_.xPos, draw_area_.yPos - draw_area_.height, draw_area_.width, draw_area_.height, 0, 0, draw_area_.width, draw_area_.height, draw_area_.data, &draw_area_.bmi, DIB_RGB_COLORS, SRCCOPY);
	draw_area_.update = false;
}

void Renderer::ClearRenderArea(bool force, int panel, uint32_t p_colour) {

	if (!force) {
		if (!draw_area_.update)
			return;
	}

	/* 
	* Starting from first element in data struct (corresponds to bottom left pixel),
	* iterate and set all bits to colour corresponding to panel in which they are located.
	*/
	uint32_t* pixel = (uint32_t*)draw_area_.data;
	uint32_t colour = bg_colour_passive;
	uint16_t pixel_count, width;
	Point current_pixel;

	// clear entire buffer
	if (panel == -1) {

		for (int i = 0; i < draw_area_.height; i++) {
			pixel_count = 0;
			width = 0;
			for (int j = 0; j < draw_area_.width; j++) {
				// iterate over known width of draw panel instead of verifying collision again
				if (pixel_count < width) {
					pixel_count++;
					*pixel++ = colour;
					continue;
				}
				width = 0;

				// once initial collision is found, set colour and track counter to determine how many subsequent pixels are required of the same colour
				current_pixel = Point(j, draw_area_.height - i);
				if (draw_area_.aabb[TOP_DOWN].Collision(current_pixel)) {
					pixel_count = 0;
					width = draw_area_.aabb[TOP_DOWN].RB.x - draw_area_.aabb[TOP_DOWN].LT.x;
					draw_area_.focus == panel ? colour = td_colour_active : colour = td_colour_passive;
					*pixel++ = colour;
					continue;
				}
				if (draw_area_.aabb[FIRST_PERSON].Collision(current_pixel)) {
					pixel_count = 0;
					width = draw_area_.aabb[FIRST_PERSON].RB.x - draw_area_.aabb[FIRST_PERSON].LT.x;
					draw_area_.focus == panel ? colour = fp_colour_active : colour = fp_colour_passive;
					*pixel++ = colour;
					continue;
				}
				*pixel++ = bg_colour_passive;
			}
		}
	}
	// clear only the panel specified
	else {
		pixel += draw_area_.width * (draw_area_.aabb[BACKGROUND].RB.y - draw_area_.aabb[panel].RB.y) + (draw_area_.aabb[panel].LT.x - draw_area_.aabb[BACKGROUND].LT.x);

		switch (panel) {
		case TOP_DOWN:
		{
			draw_area_.focus == panel ? colour = td_colour_active : colour = td_colour_passive;
			colour = td_colour_passive;
		} break;
		case FIRST_PERSON:
		{
			draw_area_.focus == panel ? colour = fp_colour_active : colour = fp_colour_passive;
			colour = fp_colour_passive;
		} break;
		case BACKGROUND:
		{
			return;
		}
		}

		for (int i = 0; i < (draw_area_.aabb[panel].RB.y - draw_area_.aabb[panel].LT.y); i++) {
			for (int j = 0; j < (draw_area_.aabb[panel].RB.x - draw_area_.aabb[panel].LT.x); j++) {
				*pixel++ = colour;
			}
			pixel += draw_area_.width - (draw_area_.aabb[panel].RB.x - draw_area_.aabb[panel].LT.x);
		}
	}

	draw_area_.update = true;
}

void Renderer::SetFocus(int panel) {
	draw_area_.focus = panel;
}

void Renderer::CleanUp() {

	if (!this)
		return;
	if (draw_area_.data)
		free(draw_area_.data);
}

Point Renderer::Clamp(Point &p, Point max) {

	if (p.x >= max.x)		p.x = max.x;
	if (p.x < 0)			p.x = 0;
	if (p.y >= max.y)		p.y = max.y;
	if (p.y < 0)			p.y = 0;

	return p;
}

uint32_t* Renderer::GetMemoryLocation(int panel, Point p) {

	uint32_t cursorMemoryLocation = (uint32_t)draw_area_.data + p.x + p.y * draw_area_.width;
	return &cursorMemoryLocation;
}

bool Renderer::Validate(int panel, Point p) {

	return (p.x >= 0 &&	p.x < draw_area_.width && p.y >= 0 && p.y < draw_area_.height);
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