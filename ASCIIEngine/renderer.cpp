#include "renderer.h"

Renderer::Renderer(Rect* draw_rect, uint8_t border_width) {

	if (instanced)
		return;
	instanced = true;

	draw_area_ = DrawArea();
}

Renderer::DrawArea* Renderer::GetDrawArea() {
	return &draw_area_;
}

void Renderer::SetDrawArea(Rect* rect_inc, uint8_t border_width) {

	/*
	* A draw_area_ object contains information regarding each panel that will be drawn to screen.
	* draw_area_.panels[BACKGROUND] is the entire range of the client rect { 1604, 561 } zero indexed.
	* draw_area_.panels[TOP_DOWN] is the range from:		LT = (dr.LT.x + BW, dr.LT.y + BW)						BR = ((dr.LT.x + dr.RB.x) / 2 - BW, dr.RB.y - BW)
	* draw_area_.panels[FIRST_PERSON] is the range from		LT = ((dr.LT.x + dr.RB.x) / 2 + BW, dr.LT.y + BW)		BR = (dr.RB.x - BW, dr.RB.y - BW)
	* Using WindowRect dimensions of 1620 by 800, yields total area of 1604W x 561H, zero indexed.
	*/
	Rect rect = *rect_inc;
	uint32_t data_size = 0;

	draw_area_.xPos = rect.LT.x;
	draw_area_.yPos = rect.RB.y;
	draw_area_.width = rect.GetWidth();
	draw_area_.height = rect.GetHeight();

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

	draw_area_.panels[TOP_DOWN] = Rect(Point2d(rect.LT.x + border_width, rect.LT.y + border_width), Point2d(rect.GetWidth() / 2 - border_width, rect.RB.y - border_width));
	draw_area_.panels[FIRST_PERSON] = Rect(Point2d(rect.GetWidth() / 2 + border_width, rect.LT.y + border_width), Point2d(rect.RB.x - border_width, rect.RB.y - border_width));
	draw_area_.panels[BACKGROUND] = Rect(rect);
}

void Renderer::UpdateRenderArea(Point2d p, int panel, uint32_t colour, bool valid) {

	if (!valid)
		if (!Validate(p, panel))
			return;

	// Draws from bottom left to top right as first memory location indicates bottom left corner
	uint32_t* pixel = (uint32_t*)draw_area_.data + (draw_area_.width * draw_area_.height) - (p.y * draw_area_.width) + p.x;
		*pixel = colour;
	
	draw_area_.update = true;
}

void Renderer::UpdateRenderArea(Line l, int panel, uint32_t colour, bool valid) {
	
	// trivial case
	if (l.a == l.b)
		return;
	
	if (!valid)
		if (!Validate(l, panel))
			// TODO: implement clip line instead of simple return
			return;

	// Special case for vertical lines
	if (l.a.x == l.b.x) {
		Point2d p;
		l.a.y < l.b.y ? p = l.a : p = l.b;

		int limit = abs((int)(l.a.y - l.b.y));
		for (int i = 0; i < limit; i++) {
			// TODO: valid flag will need to account for clipping (maybe alter line object sent for rendering to ensure all points reside in viewport?)
			UpdateRenderArea(p, panel, colour, true);
			p.y++;
		}
		return;
	}
	// Special case for horizontal lines
	if (l.a.y == l.b.y) {
		Point2d p;
		l.a.x < l.b.x ? p = l.a : p = l.b;

		int limit = abs((int)(l.a.x - l.b.x));
		for (int i = 0; i < limit; i++) {
			// TODO: valid flag will need to account for clipping (maybe alter line object sent for rendering to ensure all points reside in viewport?)
			UpdateRenderArea(p, panel, colour, true);
			p.x++;
		}
		return;
	}

	// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	if (abs((int)(l.b.y - l.a.y)) < abs((int)(l.b.x - l.a.x)))
		l.a.x > l.b.x ? RenderLineLow(l.b, l.a, panel, colour, true) : RenderLineLow(l.a, l.b, panel, colour, true);
	else
		l.a.y > l.b.y ? RenderLineHigh(l.b, l.a, panel, colour, true) : RenderLineHigh(l.a, l.b, panel, colour, true);

	draw_area_.update = true;
}

void Renderer::RenderLineLow(Point2d p0, Point2d p1, int panel, uint32_t colour, bool valid) {
	// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	int yi = 1;
	if (dy < 0) {
		yi = -1;
		dy = -dy;
	}
	int D = (2 * dy) - dx;
	uint32_t y = p0.y;

	for (uint32_t x = p0.x; x <= p1.x; x++) {
		UpdateRenderArea(Point2d(x, y), panel, colour, true);
		if (D > 0) {
			y += yi;
			D += (2 * (dy - dx));
		} else
			D += (2 * dy);
	}
}

void Renderer::RenderLineHigh(Point2d p0, Point2d p1, int panel, uint32_t colour, bool valid) {
	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	int xi = 1;
	if (dx < 0) {
		xi = -1;
		dx = -dx;
	}
	int D = (2 * dx) - dy;
	uint32_t x = p0.x;

	for (uint32_t y = p0.y; y <= p1.y; y++) {
		UpdateRenderArea(Point2d(x, y), panel, colour, true);
		if (D > 0) {
			x += xi;
			D += (2 * (dx - dy));
		} else
			D += (2 * dx);
	}
}

void Renderer::UpdateRenderArea(Rect p_rect, int panel, uint32_t colour, bool valid) {
	// Used for drawing UI elements, since they will typically be the only objects represented as rectangles
	if (!valid)
		if (!Validate(p_rect, panel))
			// TODO: implement clip line instead of simple return
			return;

	// for now, assuming that no rasterization is required (all lines are horizontal or vertical)
	Rect rect = p_rect;
	// not optimized for fewest iterations (ie. does not determine whether it is more efficient to do length then width or vice versa)
	Point2d p = { rect.LT.x, rect.RB.y };
	for (int i = rect.RB.y; i > rect.LT.y; i--) {
		for (int j = rect.LT.x; j <= rect.RB.x; j++) {
			UpdateRenderArea(p, panel, colour, true);
			p.x++;
		}
		p.x = rect.LT.x;
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
	Point2d current_pixel;

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

				current_pixel = Point2d(j, draw_area_.height - i);
				if (draw_area_.panels[TOP_DOWN].Collision(current_pixel)) {
					// set counter to 0 and width to width of one horizontal chunk of panel for reiteration above
					pixel_count = 0;
					width = draw_area_.panels[TOP_DOWN].GetWidth();
					// set colour depending on cursor location and whether the panel focus has been locked
					colour = td_colour_passive;
					if (draw_area_.lock_focus == TOP_DOWN || (draw_area_.focus == TOP_DOWN && draw_area_.lock_focus == -1)) {
						colour = td_colour_active;
					}
					*pixel++ = colour;
					continue;
				}
				if (draw_area_.panels[FIRST_PERSON].Collision(current_pixel)) {
					// set counter to 0 and width to width of one horizontal chunk of panel for reiteration above
					pixel_count = 0;
					width = draw_area_.panels[FIRST_PERSON].GetWidth();
					// set colour depending on cursor location and whether the panel focus has been locked
					colour = fp_colour_passive;
					if (draw_area_.lock_focus == FIRST_PERSON || (draw_area_.focus == FIRST_PERSON && draw_area_.lock_focus == -1)) {
						colour = fp_colour_active;
					}
					*pixel++ = colour;
					continue;
				}
				*pixel++ = bg_colour_passive;
			}
		}
	}
	// clear only the panel specified
	else {
		pixel += draw_area_.width * (draw_area_.panels[BACKGROUND].RB.y - draw_area_.panels[panel].RB.y) + (draw_area_.panels[panel].LT.x - draw_area_.panels[BACKGROUND].LT.x);

		switch (panel) {
		case TOP_DOWN:
		{
			draw_area_.focus == panel ? colour = colours[TOP_DOWN][1] : colour = colours[TOP_DOWN][0];
			//colour = td_colour_passive;
		} break;
		case FIRST_PERSON:
		{
			draw_area_.focus == panel ? colour = colours[FIRST_PERSON][1] : colour = colours[FIRST_PERSON][0];
			//draw_area_.focus == panel ? colour = fp_colour_active : colour = fp_colour_passive;
			//colour = fp_colour_passive;
		} break;
		case BACKGROUND:
		{
			return;
		}
		}

		for (int i = 0; i < (draw_area_.panels[panel].RB.y - draw_area_.panels[panel].LT.y); i++) {
			for (int j = 0; j < (draw_area_.panels[panel].RB.x - draw_area_.panels[panel].LT.x); j++) {
				*pixel++ = colour;
			}
			pixel += draw_area_.width - (draw_area_.panels[panel].RB.x - draw_area_.panels[panel].LT.x);
		}
	}

	draw_area_.update = true;
}

int Renderer::GetFocus() {

	return draw_area_.focus;
}

void Renderer::SetFocus(int panel) {

	if (draw_area_.lock_focus != -1)
		return;
	draw_area_.focus = panel;
	draw_area_.update = true;
}

int Renderer::GetFocusLock() {

	return draw_area_.lock_focus;
}

void Renderer::SetFocusLock(int panel) {

	draw_area_.update = true;
	if (draw_area_.lock_focus == -1) {
		draw_area_.lock_focus = panel;
		return;
	}
	draw_area_.lock_focus = -1;
}

void Renderer::CleanUp() {

	if (!this)
		return;
	if (draw_area_.data)
		free(draw_area_.data);
}

Point2d Renderer::Clamp(Point2d &p, Point2d max) {

	if (p.x >= max.x)		p.x = max.x;
	if (p.x < 0)			p.x = 0;
	if (p.y >= max.y)		p.y = max.y;
	if (p.y < 0)			p.y = 0;

	return p;
}

uint32_t* Renderer::GetMemoryLocation(int panel, Point2d p) {

	uint32_t* cursorMemoryLocation = (uint32_t*)draw_area_.data + p.x + p.y * draw_area_.width;
	return cursorMemoryLocation;
}

bool Renderer::Validate(Point2d p, int panel) {

	if (panel == -1)
		return (p.x >= 0 && p.x < draw_area_.width&& p.y >= 0 && p.y < draw_area_.height);
	return (p.x >= draw_area_.panels[panel].LT.x && p.x < draw_area_.panels[panel].RB.x && p.y >= draw_area_.panels[panel].LT.y && p.y < draw_area_.panels[panel].RB.y);
}

bool Renderer::Validate(Line l, int panel) {

	return (Validate(l.a, panel) && Validate(l.b, panel));
}

bool Renderer::Validate(Rect rect, int panel) {

	return (Validate(rect.LT, panel) && Validate(rect.RB, panel));
}

void Renderer::DrawLine(Point2d p1, Point2d p2, int weight, uint32_t colour)
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
	
	if (!(Validate(p1, 1) && Validate(p2, 1)))
	{

		ClipLine(Line{ p1, p2 });
	}

	

	Point2d* min_y = nullptr;
	Point2d* min_x = nullptr;
	Point2d* max_y = nullptr;
	Point2d* max_x = nullptr;
	
	
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
	return Line{ Point2d {}, Point2d {} };
}