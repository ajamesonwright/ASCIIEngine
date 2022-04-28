#include "renderer.h"

Renderer::Renderer(Rect* draw_rect, uint8_t border_width) {

	if (instanced)
		return;
	instanced = true;

	draw_area_ = DrawArea();
	this->fov = 60;
}

Renderer::DrawArea* Renderer::GetDrawArea() {
	return &draw_area_;
}

void Renderer::SetDrawArea(Rect* rect_inc, uint8_t border_width) {

	/*
	* A draw_area_ object contains information regarding each panel that will be drawn to screen.
	* draw_area_.panels[BACKGROUND] is the entire range of the client rect { 1604, 561 } zero indexed.
	* draw_area_.panels[TOP_DOWN] is the range from:		lt = (dr.lt.x + BW, dr.lt.y + BW + 1)						BR = ((dr.lt.x + dr.rb.x) / 2 - BW, dr.rb.y - BW)
	* draw_area_.panels[FIRST_PERSON] is the range from		lt = ((dr.lt.x + dr.rb.x) / 2 + BW, dr.lt.y + BW + 1)		BR = (dr.rb.x - BW, dr.rb.y - BW)
	* Using WindowRect dimensions of 1620 by 800, yields total area of 1604W x 561H, zero indexed.
	*/
	Rect rect = *rect_inc;
	uint32_t data_size = 0;

	draw_area_.xPos = rect.lt.x;
	draw_area_.yPos = rect.rb.y;
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

	// Add 1 additional pixel to top point coordinate to make sure we're working with even numbers for height
	draw_area_.panels[TOP_DOWN] = Rect(Point2d(rect.lt.x + border_width, rect.lt.y + border_width + 1), Point2d(rect.GetWidth() / 2 - border_width, rect.rb.y - border_width));
	draw_area_.panels[FIRST_PERSON] = Rect(Point2d(rect.GetWidth() / 2 + border_width, rect.lt.y + border_width + 1), Point2d(rect.rb.x - border_width, rect.rb.y - border_width));
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

void Renderer::UpdateRenderArea(Ray2d r, int panel, uint32_t colour, bool valid) {
	if (!valid)
		if (!Validate(r, panel))
			return;

	// Draw main line
	Point2d base, tip;
	double cosx = cos(r.dir * M_PI / 180);
	double siny = sin(r.dir * M_PI / 180);
	base = Point2d((uint32_t)(r.x - r.size * cosx / 2 + 0.5), (uint32_t)(r.y - r.size * siny / 2 + 0.5));
	tip = Point2d((uint32_t)(r.x + r.size * cosx / 2 + 0.5), (uint32_t)(r.y + r.size * siny / 2 + 0.5));
	Line main = Line(base, tip);
	UpdateRenderArea(main, panel, colour, true);

	// Draw edges of arrow
	Point2d left, right;
	left = Point2d((uint32_t)(tip.x - 0.30 * r.size * cos((r.dir + 180 - 135) * M_PI / 180) + 0.5), (uint32_t)(tip.y - 0.30 * r.size * sin((r.dir + 180 - 135) * M_PI / 180) + 0.5));
	right = Point2d((uint32_t)(tip.x - 0.30 * r.size * cos((r.dir + 180 + 135) * M_PI / 180) + 0.5), (uint32_t)(tip.y - 0.30 * r.size * sin((r.dir + 180 + 135) * M_PI / 180) + 0.5));
	Line left_edge = Line(tip, left);
	Line right_edge = Line(tip, right);
	UpdateRenderArea(left_edge, panel, colour, true);
	UpdateRenderArea(right_edge, panel, colour, true);
}

void Renderer::UpdateRenderArea(Geometry g, int panel, uint32_t colour, bool valid) {

	switch (g.type) {
	case Geometry::G_LINE:
	{
		UpdateRenderArea(static_cast<Line>(g), 0);
	} break;
	case Geometry::G_TRI:
	{
		Line l0 = Line(*g.vertices.at(0), *g.vertices.at(1));
		Line l1 = Line(*g.vertices.at(1), *g.vertices.at(2));
		Line l2 = Line(*g.vertices.at(2), *g.vertices.at(0));
		UpdateRenderArea(l0, panel, colour, valid);
		UpdateRenderArea(l1, panel, colour, valid);
		UpdateRenderArea(l2, panel, colour, valid);
		//UpdateRenderArea(static_cast<Tri>(g), 0);
	} break;
	case Geometry::G_RECT:
	{
		UpdateRenderArea(static_cast<Rect>(g), 0);
	} break;
	}
}

void Renderer::UpdateRenderArea(Line l, int panel, uint32_t colour, bool valid) {
	
	// trivial case
	if (l.vertices.at(0) == l.vertices.at(1))
		return;
	
	if (!valid)
		if (!Validate(l, panel))
			// TODO: implement clip line instead of simple return
			return;

	// Special case for vertical lines
	if (l.vertices.at(0)->x == l.vertices.at(1)->x) {
		Point2d p;
		l.vertices.at(0)->y < l.vertices.at(1)->y ? p = *l.vertices.at(0) : p = *l.vertices.at(1);

		int limit = abs((int)(l.vertices.at(0)->y - l.vertices.at(1)->y));
		for (int i = 0; i < limit; i++) {
			// TODO: valid flag will need to account for clipping (maybe alter line object sent for rendering to ensure all points reside in viewport?)
			UpdateRenderArea(p, panel, colour, true);
			p.y++;
		}
		return;
	}
	// Special case for horizontal lines
	if (l.vertices.at(0)->y == l.vertices.at(1)->y) {
		Point2d p;
		l.vertices.at(0)->x < l.vertices.at(1)->x ? p = *l.vertices.at(0) : p = *l.vertices.at(1);

		int limit = abs((int)(l.vertices.at(0)->x - l.vertices.at(1)->x));
		for (int i = 0; i < limit; i++) {
			// TODO: valid flag will need to account for clipping (maybe alter line object sent for rendering to ensure all points reside in viewport?)
			UpdateRenderArea(p, panel, colour, true);
			p.x++;
		}
		return;
	}

	// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	if (abs((int)(l.vertices.at(1)->y - l.vertices.at(0)->y)) < abs((int)(l.vertices.at(1)->x - l.vertices.at(0)->x)))
		l.vertices.at(0)->x > l.vertices.at(1)->x ? RenderLineLow(*l.vertices.at(1), *l.vertices.at(0), panel, colour, true) : RenderLineLow(*l.vertices.at(0), *l.vertices.at(1), panel, colour, true);
	else
		l.vertices.at(0)->y > l.vertices.at(1)->y ? RenderLineHigh(*l.vertices.at(1), *l.vertices.at(0), panel, colour, true) : RenderLineHigh(*l.vertices.at(0), *l.vertices.at(1), panel, colour, true);

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
	Point2d p = { rect.lt.x, rect.rb.y };
	for (uint32_t i = rect.rb.y; i >= rect.lt.y; i--) {
		for (uint32_t j = rect.lt.x; j <= rect.rb.x; j++) {
			UpdateRenderArea(p, panel, colour, true);
			p.x++;
		}
		p.x = rect.lt.x;
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
	uint32_t colour = colours[BACKGROUND][0];
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
					colour = colours[TOP_DOWN][0];
					if (draw_area_.lock_focus == TOP_DOWN || (draw_area_.focus == TOP_DOWN && draw_area_.lock_focus == -1)) {
						colour = colours[TOP_DOWN][1];
					}
					*pixel++ = colour;
					continue;
				}
				if (draw_area_.panels[FIRST_PERSON].Collision(current_pixel)) {
					// set counter to 0 and width to width of one horizontal chunk of panel for reiteration above
					pixel_count = 0;
					width = draw_area_.panels[FIRST_PERSON].GetWidth();
					// set colour depending on cursor location and whether the panel focus has been locked
					colour = colours[FIRST_PERSON][0];
					if (draw_area_.lock_focus == FIRST_PERSON || (draw_area_.focus == FIRST_PERSON && draw_area_.lock_focus == -1)) {
						colour = colours[FIRST_PERSON][1];
					}
					*pixel++ = colour;
					continue;
				}
				*pixel++ = colours[BACKGROUND][0];
			}
		}
	}
	// clear only the panel specified
	else {
		pixel += draw_area_.width * (draw_area_.panels[BACKGROUND].rb.y - draw_area_.panels[panel].rb.y) + (draw_area_.panels[panel].lt.x - draw_area_.panels[BACKGROUND].lt.x);

		switch (panel) {
		case TOP_DOWN:
		{
			draw_area_.focus == panel ? colour = colours[TOP_DOWN][1] : colour = colours[TOP_DOWN][0];
		} break;
		case FIRST_PERSON:
		{
			draw_area_.focus == panel ? colour = colours[FIRST_PERSON][1] : colour = colours[FIRST_PERSON][0];
		} break;
		case BACKGROUND:
		{
			return;
		}
		}

		for (int i = 0; i < (draw_area_.panels[panel].rb.y - draw_area_.panels[panel].lt.y); i++) {
			for (int j = 0; j < (draw_area_.panels[panel].rb.x - draw_area_.panels[panel].lt.x); j++) {
				*pixel++ = colour;
			}
			pixel += draw_area_.width - (draw_area_.panels[panel].rb.x - draw_area_.panels[panel].lt.x);
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

void* Renderer::GetMemoryLocation(int panel, Point2d p) {

	uint32_t* cursorMemoryLocation = (uint32_t*)draw_area_.data + (draw_area_.width * draw_area_.height) - (p.y * draw_area_.width) + p.x;
	return cursorMemoryLocation;
}

bool Renderer::Validate(Point2d p, int panel) {

	if (panel == -1)
		return (p.x >= 0 && p.x < draw_area_.width&& p.y >= 0 && p.y < draw_area_.height);
	return (p.x >= draw_area_.panels[panel].lt.x && p.x < draw_area_.panels[panel].rb.x && p.y >= draw_area_.panels[panel].lt.y && p.y < draw_area_.panels[panel].rb.y);
}

bool Renderer::Validate(Ray2d r, int panel) {
	
	double cosx = cos(r.dir * M_PI / 180);
	double siny = sin(r.dir * M_PI / 180);

	if (panel == -1)
		return (r.x - (r.size / 2 * cosx) >= 0
			&& r.x + (r.size / 2 * cosx) < draw_area_.width
			&& r.y - (r.size / 2 * siny) >= 0 
			&& r.y + (r.size / 2 * siny) < draw_area_.height);

	return (r.x - (r.size / 2 * cosx) >= draw_area_.panels[panel].lt.x
		&& r.x + (r.size / 2 * cosx) < draw_area_.panels[panel].rb.x
		&& r.y - (r.size / 2 * siny) >= draw_area_.panels[panel].lt.y
		&& r.y + (r.size / 2 * siny) < draw_area_.panels[panel].rb.y);
}

bool Renderer::Validate(Line l, int panel) {

	return (Validate(*l.vertices.at(0), panel) && Validate(*l.vertices.at(1), panel));
}

bool Renderer::Validate(Rect rect, int panel) {

	return (Validate(rect.lt, panel) && Validate(rect.rb, panel));
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