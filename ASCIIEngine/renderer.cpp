#include "renderer.h"
#include "character_set.h"
#include <iostream>

Renderer::Renderer(Rect* drawRect, uint8_t borderWidth) {

	if (instanced)
		return;
	instanced = true;

	drawArea = DrawArea();
}

Renderer::DrawArea::DrawArea() {
	xPos = 0;
	yPos = 0;
	width = 0;
	height = 0;
	data = nullptr;
	/*for (void* p : data) {
		p = nullptr;
	}*/
	focus = -1;
	lockFocus = -1;
	update = false;
}

Renderer::DrawArea::DrawArea(uint32_t p_xPos, uint32_t p_yPos, uint32_t p_width, uint32_t p_height) {
	xPos = p_xPos;
	yPos = p_yPos;
	width = p_width;
	height = p_height;

	uint32_t data_size = (width * height) * sizeof(uint32_t);
	/*for (int i = 0; i < NUM_BUFFERS; i++) {
		data[i] = malloc(data_size);
	}*/
	data = malloc(data_size);

	update = true;
	focus = -1;
	lockFocus = -1;

	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
}

void Renderer::init(const Camera& camera) {
	for (int i = 0; i < THREAD_COUNT; i++) {
		threadPool[FIRST_PERSON][i] = std::thread(updateRenderAreaFunc, this, camera, i);
	}
}

Renderer::DrawArea* Renderer::getDrawArea() {
	return &drawArea;
}

Rect Renderer::getDrawArea(int panelId) {
	return drawArea.panels[panelId];
}

void Renderer::setDrawArea(Rect* rectInc, uint8_t borderWidth) {

	/*
	* A drawArea object contains information regarding each panel that will be drawn to screen.
	* drawArea.panels[BACKGROUND] is the entire range of the client rect { 1604, 561 } zero indexed.
	* drawArea.panels[TOP_DOWN] is the range from:		lt = (dr.lt.x + BW, dr.lt.y + BW + 1)						BR = ((dr.lt.x + dr.rb.x) / 2 - BW, dr.rb.y - BW)
	* drawArea.panels[FIRST_PERSON] is the range from		lt = ((dr.lt.x + dr.rb.x) / 2 + BW, dr.lt.y + BW + 1)		BR = (dr.rb.x - BW, dr.rb.y - BW)
	* Using WindowRect dimensions of 1620 by 800, yields total area of 1604W x 561H, zero indexed.
	*/
	Rect rect = *rectInc;
	uint32_t dataSize = 0;

	drawArea.xPos = rect.lt.x;
	drawArea.yPos = rect.rb.y;
	drawArea.width = rect.getWidth();
	drawArea.height = rect.getHeight();

	dataSize = (drawArea.width * drawArea.height) * sizeof(uint32_t);

	drawArea.update = true;

	// Clear on resize when already full
	/*for (int i = 0; i < NUM_BUFFERS; i++) {
		if (drawArea.data[i]) {
			free(drawArea.data[i]);
		}
		drawArea.data[i] = malloc(dataSize);
	}*/
	drawArea.data = malloc(dataSize);

	drawArea.bmi.bmiHeader.biSize = sizeof(drawArea.bmi.bmiHeader);
	drawArea.bmi.bmiHeader.biWidth = drawArea.width;
	drawArea.bmi.bmiHeader.biHeight = drawArea.height;
	drawArea.bmi.bmiHeader.biPlanes = 1;
	drawArea.bmi.bmiHeader.biBitCount = 32;
	drawArea.bmi.bmiHeader.biCompression = BI_RGB;

	// Add 1 additional pixel to top point coordinate to make sure we're working with even numbers for height
	drawArea.panels[TOP_DOWN] = Rect(Point2d(rect.lt.x + borderWidth, rect.lt.y + borderWidth + 1), Point2d(rect.getWidth() / 2 - borderWidth, rect.rb.y - borderWidth));
	drawArea.panels[FIRST_PERSON] = Rect(Point2d(rect.getWidth() / 2 + borderWidth, rect.lt.y + borderWidth + 1), Point2d(rect.rb.x - borderWidth, rect.rb.y - borderWidth));
	drawArea.panels[BACKGROUND] = Rect(rect);
}

void Renderer::clampDimension(uint32_t& dim, Dimension d, int panel) {
	uint32_t screenDim = d == HORIZONTAL ? drawArea.screensWidth : drawArea.screensHeight;
	uint32_t lower = d == HORIZONTAL ? drawArea.panels[panel].lt.x : drawArea.panels[panel].lt.y;
	uint32_t upper = d == HORIZONTAL ? drawArea.panels[panel].rb.x : drawArea.panels[panel].rb.y;
	clampDimension(dim, lower, upper, screenDim);
}

uint16_t Renderer::validate(Geometry* g, uint32_t bounds[], int panel) {
	uint16_t result = 0b0;
	switch (g->type) {
	case Geometry::G_LINE:
	{
		result = validate(static_cast<Line>(*g), bounds, panel);
	} break;
	/*case Geometry::G_TRI:
	{
		return validate(static_cast<Tri>(g), bounds, panel);
	} break;*/
	case Geometry::G_RECT:
	{
		result = validate(static_cast<Rect>(*g), bounds, panel);
	} break;
	case Geometry::G_CIRCLE:
	{
		result = validate(static_cast<Circle>(*g), bounds, panel);
	} break;
	}
	return result;
}

void Renderer::updateRenderArea(const Point2d& p, int panel, uint32_t colour, bool valid) {
	uint32_t bounds[4] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX }; // minX, maxX, minY, maxY
	if (!valid) {
		uint16_t clipType = validate(p, bounds, panel);
		if (clipType)
			return;
	}

	// Draws from bottom left to top right as first memory location indicates bottom left corner
	uint32_t* pixel = (uint32_t*)drawArea.data + (drawArea.width * drawArea.height) - (p.y * drawArea.width) + p.x;
		*pixel = colour;
	
	drawArea.update = true;
}

void Renderer::updateRenderArea(const Camera& c, int panel, uint32_t colour, bool valid) {
	Line clippedLine;
	uint32_t bounds[4] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX }; // minX, maxX, minY, maxY
	if (!valid) {
		if (!validate(c, bounds, panel)) {

		}
	}

	// Draw main line
	Line main(c.base, c.tip);
	updateRenderArea(main, panel, colour, true);

	// Draw edges of arrow
	Line leftEdge(c.tip, c.left);
	updateRenderArea(leftEdge, panel, colour, true);
	Line rightEdge(c.tip, c.right);
	updateRenderArea(rightEdge, panel, colour, true);
}

void Renderer::updateRenderArea(Geometry* g, int panel, uint32_t colour, bool valid) {
	switch (g->type) {
	case Geometry::G_LINE:
	{
		updateRenderArea(static_cast<Line>(*g), 0);
	} break;
	case Geometry::G_TRI:
	{
		Line l0 = Line(g->vertices.at(0), g->vertices.at(1));
		Line l1 = Line(g->vertices.at(1), g->vertices.at(2));
		Line l2 = Line(g->vertices.at(2), g->vertices.at(0));
		updateRenderArea(l0, panel, colour, valid);
		updateRenderArea(l1, panel, colour, valid);
		updateRenderArea(l2, panel, colour, valid);
		//UpdateRenderArea(static_cast<Tri>(g), 0);
	} break;
	case Geometry::G_RECT:
	{
		updateRenderArea(static_cast<Rect>(*g), 0);
	} break;
	case Geometry::G_CIRCLE:
	{
		updateRenderArea(static_cast<Circle>(*g), 0);
	} break;
	}
}

void Renderer::updateRenderArea(const Line& l, int panel, uint32_t colour, bool valid) {
	// null case
	if (l.vertices.size() < 2)
		return;
	// trivial case
	if (l.vertices.at(0) == l.vertices.at(1))
		return;
	
	Line clippedLine = l;
	uint32_t bounds[4] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX }; // minX, maxX, minY, maxY
	if (!valid) {
		uint16_t clipType = validate(l, bounds, panel);
		if (clipType) {
			clippedLine = clipLine(l, bounds, clipType, panel);
		}
	}

	// Special case for vertical lines
	if (clippedLine.vertices.at(0).x == clippedLine.vertices.at(1).x) {
		Point2d p;
		clippedLine.vertices.at(0).y < clippedLine.vertices.at(1).y ? p = clippedLine.vertices.at(0) : p = clippedLine.vertices.at(1);

		int limit = abs((int)(clippedLine.vertices.at(0).y - clippedLine.vertices.at(1).y));
		for (int i = 0; i < limit; i++) {
			updateRenderArea(p, panel, colour, true);
			p.y++;
		}
		return;
	}
	// Special case for horizontal lines
	if (clippedLine.vertices.at(0).y == clippedLine.vertices.at(1).y) {
		Point2d p;
		clippedLine.vertices.at(0).x < clippedLine.vertices.at(1).x ? p = clippedLine.vertices.at(0) : p = clippedLine.vertices.at(1);

		int limit = abs((int)(clippedLine.vertices.at(0).x - clippedLine.vertices.at(1).x));
		for (int i = 0; i < limit; i++) {
			updateRenderArea(p, panel, colour, true);
			p.x++;
		}
		return;
	}

	// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	if (abs((int)(clippedLine.vertices.at(1).y - clippedLine.vertices.at(0).y)) < abs((int)(clippedLine.vertices.at(1).x - clippedLine.vertices.at(0).x)))
		clippedLine.vertices.at(0).x > clippedLine.vertices.at(1).x ? renderLineLow(clippedLine.vertices.at(1), clippedLine.vertices.at(0), panel, colour, true) : renderLineLow(clippedLine.vertices.at(0), clippedLine.vertices.at(1), panel, colour, true);
	else
		clippedLine.vertices.at(0).y > clippedLine.vertices.at(1).y ? renderLineHigh(clippedLine.vertices.at(1), clippedLine.vertices.at(0), panel, colour, true) : renderLineHigh(clippedLine.vertices.at(0), clippedLine.vertices.at(1), panel, colour, true);

	drawArea.update = true;
}


void Renderer::renderLineLow(const Point2d& p0, const Point2d& p1, int panel, uint32_t colour, bool valid) {
	// If dx > dy, increment over x value to determine y value
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
		updateRenderArea(Point2d(x, y), panel, colour, true);
		if (D > 0) {
			y += yi;
			D += (2 * (dy - dx));
		} else
			D += (2 * dy);
	}
}

void Renderer::renderLineHigh(const Point2d& p0, const Point2d& p1, int panel, uint32_t colour, bool valid) {
	// If dx < dy, increment over y value to determine x value
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
		updateRenderArea(Point2d(x, y), panel, colour, true);
		if (D > 0) {
			x += xi;
			D += (2 * (dx - dy));
		} else
			D += (2 * dx);
	}
}

void Renderer::updateRenderArea(const Tri& tri, int panel, uint32_t colour, bool valid) {

}

void Renderer::updateRenderArea(const Rect& rect, int panel, uint32_t colour, bool valid) {
	// Used for drawing UI elements, since they will typically be the only objects represented as rectangles
	
	Rect clippedRect = rect;
	uint32_t bounds[4] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX }; // minX, maxX, minY, maxY
	if (!valid) {
		uint16_t clipType = validate(rect, bounds, panel);
		if (clipType) {
			clippedRect = clipRect(rect, bounds, clipType, panel);
		}
	}

	// for now, assuming that no rasterization is required (all lines are horizontal or vertical)
	Point2d p = { clippedRect.lt.x, clippedRect.rb.y };
	for (uint32_t i = clippedRect.rb.y; i >= clippedRect.lt.y; i--) {
		for (uint32_t j = clippedRect.lt.x; j <= clippedRect.rb.x; j++) {
			updateRenderArea(p, panel, colour, true);
			p.x++;
		}
		p.x = clippedRect.lt.x;
		p.y--;
	}

	drawArea.update = true;
}

void Renderer::updateRenderArea(const Circle& circle, int panel, uint32_t colour, bool valid) {
	uint32_t bounds[4] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX }; // minX, maxX, minY, maxY
	if (!valid) {
		uint16_t clipType = validate(circle, bounds, panel);
		if (clipType) {

		}
	}

	Circle c = circle;

	for (Point2d v : c.vertices) {
		updateRenderArea(v, panel, colour, true);
	}
}

void Renderer::drawRenderArea(HDC hdc) {
	if (!drawArea.update)
		return;

	// Draws selected DrawArea from top left to bottom right
	StretchDIBits(hdc, drawArea.xPos, drawArea.yPos - drawArea.height, drawArea.width, drawArea.height, 0, 0, drawArea.width, drawArea.height, drawArea.data, &drawArea.bmi, DIB_RGB_COLORS, SRCCOPY);
	drawArea.update = false;
}

void Renderer::clearRenderArea(Renderer* r, const bool& force, const int& panel, const uint32_t& p_colour) {
	if (!force) {
		if (!r->drawArea.update)
			return;
	}

	/* 
	* Starting from first element in data struct (corresponds to bottom left pixel),
	* iterate and set all bits to colour corresponding to panel in which they are located.
	*/
	uint32_t* pixel = (uint32_t*)r->drawArea.data;
	uint32_t colour = r->colours[BACKGROUND][0];
	uint16_t pixelCount, width;
	Point2d currentPixel;

	// clear entire buffer
	if (panel == -1) {

		for (uint32_t i = 0; i < r->drawArea.height; i++) {
			pixelCount = 0;
			width = 0;
			for (uint32_t j = 0; j < r->drawArea.width; j++) {
				// iterate over known width of draw panel instead of verifying collision again
				if (pixelCount < width) {
					pixelCount++;
					*pixel++ = colour;
					continue;
				}
				width = 0;

				currentPixel = Point2d(j, r->drawArea.height - i);
				if (r->drawArea.panels[TOP_DOWN].checkCollisionWith(currentPixel)) {
					// set counter to 0 and width to width of one horizontal chunk of panel for reiteration above
					pixelCount = 0;
					width = r->drawArea.panels[TOP_DOWN].getWidth();
					// set colour depending on cursor location and whether the panel focus has been locked
					colour = r->colours[TOP_DOWN][0];
					if (r->drawArea.lockFocus == TOP_DOWN || (r->drawArea.focus == TOP_DOWN && r->drawArea.lockFocus == -1)) {
						colour = r->colours[TOP_DOWN][1];
					}
					*pixel++ = colour;
					continue;
				}
				if (r->drawArea.panels[FIRST_PERSON].checkCollisionWith(currentPixel)) {
					// set counter to 0 and width to width of one horizontal chunk of panel for reiteration above
					pixelCount = 0;
					width = r->drawArea.panels[FIRST_PERSON].getWidth();
					// set colour depending on cursor location and whether the panel focus has been locked
					colour = r->colours[FIRST_PERSON][0];
					if (r->drawArea.lockFocus == FIRST_PERSON || (r->drawArea.focus == FIRST_PERSON && r->drawArea.lockFocus == -1)) {
						colour = r->colours[FIRST_PERSON][1];
					}
					*pixel++ = colour;
					continue;
				}
				*pixel++ = r->colours[BACKGROUND][0];
			}
		}
	}
	// clear only the panel specified
	else {
		pixel += r->drawArea.width * (r->drawArea.panels[BACKGROUND].rb.y - r->drawArea.panels[panel].rb.y) + (r->drawArea.panels[panel].lt.x - r->drawArea.panels[BACKGROUND].lt.x);

		switch (panel) {
		case TOP_DOWN:
		{
			r->drawArea.focus == panel ? colour = r->colours[TOP_DOWN][1] : colour = r->colours[TOP_DOWN][0];
		} break;
		case FIRST_PERSON:
		{
			r->drawArea.focus == panel ? colour = r->colours[FIRST_PERSON][1] : colour = r->colours[FIRST_PERSON][0];
		} break;
		case BACKGROUND:
		{
			return;
		}
		}

		for (uint32_t i = 0; i < (r->drawArea.panels[panel].rb.y - r->drawArea.panels[panel].lt.y); i++) {
			for (uint32_t j = 0; j < (r->drawArea.panels[panel].rb.x - r->drawArea.panels[panel].lt.x); j++) {
				*pixel++ = colour;
			}
			pixel += r->drawArea.width - (r->drawArea.panels[panel].rb.x - r->drawArea.panels[panel].lt.x);
		}
	}

	r->drawArea.update = true;
}

int Renderer::getFocus() {
	return drawArea.focus;
}

void Renderer::setFocus(int panel) {
	if (drawArea.lockFocus != -1)
		return;
	drawArea.focus = panel;
	drawArea.update = true;
}

int Renderer::getFocusLock() {
	return drawArea.lockFocus;
}

void Renderer::setFocusLock(int panel) {
	drawArea.update = true;
	if (drawArea.lockFocus == -1) {
		drawArea.lockFocus = panel;
		return;
	}
	drawArea.lockFocus = -1;
}

void Renderer::cleanUp() {
	if (!this)
		return;
	if (drawArea.data)
		free(drawArea.data);
}

void* Renderer::getMemoryLocation(int panel, Point2d p) {
	uint32_t* cursorMemoryLocation = (uint32_t*)drawArea.data + (drawArea.width * drawArea.height) - (p.y * drawArea.width) + p.x;
	return cursorMemoryLocation;
}

std::vector<uint8_t> Renderer::getCharacterBitmap(float brightness) {
	int index;
	try {
		index = static_cast<int>((1 - brightness) * (BRIGHTNESS_SCALE.length() - 1));
		if (brightness < 0 || brightness > 1) {
			throw std::invalid_argument("Brightness value must be in the range of 0 to 1!");
		}
	} catch (std::string e) {
		std::cout << e << std::endl;
	}
	char c = BRIGHTNESS_SCALE[index];
	return getCharacterBitmap(c);
}

uint8_t Renderer::getCharacterTileWidth() {
	return TILE_WIDTH;
}

uint8_t Renderer::getCharacterTileHeight() {
	return TILE_HEIGHT;
}

std::vector<uint8_t> Renderer::getCharacterBitmap(char c) {
	return byteArray.find(c)->second;
}

uint16_t Renderer::validate(const Point2d& p, uint32_t bounds[], int panel) {
	uint16_t result = 0b0;
	if (bounds[0] == UINT32_MAX) { // check first element to see if array is unassigned
		if (panel == -1) {
			bounds[0] = 0;
			bounds[1] = drawArea.width;
			bounds[2] = 0;
			bounds[3] = drawArea.height;
		} else {
			bounds[0] = drawArea.panels[panel].lt.x;
			bounds[1] = drawArea.panels[panel].rb.x;
			bounds[2] = drawArea.panels[panel].lt.y;
			bounds[3] = drawArea.panels[panel].rb.y;
		}
	}

	result |= (int)(p.x < bounds[0] || p.x > drawArea.screensWidth) * LEFT;
	result |= (int)(p.x > bounds[1] && p.x <= drawArea.screensWidth) * RIGHT;
	result |= (int)(p.y < bounds[2] || p.y > drawArea.screensHeight) * TOP;
	result |= (int)(p.y > bounds[3] && p.y <= drawArea.screensHeight) * BOTTOM;

	return result;
}

uint16_t Renderer::validate(const Line& l, uint32_t bounds[], int panel) {
	uint16_t result = 0b0;

	result |= validate(l.vertices.at(0), bounds, panel) << 0;
	result |= validate(l.vertices.at(1), bounds, panel) << 4;
	return result;
}

uint16_t Renderer::validate(const Rect& rect, uint32_t bounds[], int panel) {
	uint16_t result = 0b0;
	
	result |= validate(rect.lt, bounds, panel) << 0; // left and top
	result |= validate(rect.rb, bounds, panel) << 4; // right and bottom
	return result;
}

uint16_t Renderer::validate(const Circle& c, uint32_t bounds[], int panel) {
	uint16_t result = 0b0;

	result |= validate(Point2d(c.center.x - c.r, c.center.y), bounds, panel) << 0; // left
	result |= validate(Point2d(c.center.x + c.r, c.center.y), bounds, panel) << 4; // right
	result |= validate(Point2d(c.center.x, c.center.y - c.r), bounds, panel) << 8; // top
	result |= validate(Point2d(c.center.x, c.center.y + c.r), bounds, panel) << 12; // bottom

	return result;
}

uint16_t Renderer::validate(const Camera& c, uint32_t bounds[], int panel) {
	uint16_t result = 0b0;

	result |= validate(c.left, bounds, panel) << 0;
	result |= validate(c.right, bounds, panel) << 4;
	result |= validate(c.tip, bounds, panel) << 8;
	result |= validate(c.base, bounds, panel) << 12;

	return result;
}

Line Renderer::clipLine(const Line& l, const uint32_t bounds[], const uint16_t& clipType, int panel) {
	uint32_t vertexDimensions[4] = { l.vertices.at(0).x, l.vertices.at(0).y, l.vertices.at(1).x, l.vertices.at(1).y};
	int index = 0;
	int bitShift = 0;
	for (Point2d v : l.vertices) {
		uint32_t boundX = ((clipType >> bitShift & LEFT) >> 0) * bounds[0] + ((clipType >> bitShift & RIGHT) >> 1) * bounds[1];
		uint32_t boundY = ((clipType >> bitShift & TOP) >> 2) * bounds[2] + ((clipType >> bitShift & BOTTOM) >> 3) * bounds[3];
		Line line = Line(l);
		if (boundX > 0) {
			// Specify clipped Y value if the cursor is outside the left/right dims
			uint32_t clippedY = line.calculateClippedY(boundX);
			clampDimension(clippedY, bounds[2], bounds[3], drawArea.screensWidth);
			vertexDimensions[(bitShift / 4) + index + 1] = clippedY;

			if (boundY == 0) {
				vertexDimensions[(bitShift / 4) + index] = boundX;
			} else {
				// Specify clipped X value if cursor is also outside the top/bottom dims
				uint32_t clippedX = line.calculateClippedX(boundY);
				clampDimension(clippedX, bounds[0], bounds[1], drawArea.screensHeight);
				vertexDimensions[(bitShift / 4) + index] = clippedX;
			}
		}
		if (boundY > 0 && boundX == 0) {
			vertexDimensions[(bitShift / 4) + index] = line.calculateClippedX(boundY);
			vertexDimensions[(bitShift / 4) + index + 1] = boundY;
		}
		index++;
		bitShift += 4;
	}
	return Line(Point2d(vertexDimensions[0], vertexDimensions[1]), Point2d(vertexDimensions[2], vertexDimensions[3]));
}

// TODO - remove this once collisions are complete. The clipping occurs before insertion to the geometry queue
Rect Renderer::clipRect(const Rect & r, const uint32_t bounds[], const uint16_t & clipType, int panel) {
	uint32_t dimensions[4] = { r.lt.x, r.lt.y, r.rb.x, r.rb.y }; // left, top, right, bottom
	int index = 0;
	int bitShift = 0;
	for (Point2d v : r.vertices) {
		uint32_t boundX = ((clipType >> bitShift & LEFT) >> 0) * bounds[0] + ((clipType >> bitShift & RIGHT) >> 1) * bounds[1];
		uint32_t boundY = ((clipType >> bitShift & TOP) >> 2) * bounds[2] + ((clipType >> bitShift & BOTTOM) >> 3) * bounds[3];
		Line line = Line(r.lt, r.rb);
		if (boundX > 0) {
			uint32_t clippedY = line.calculateClippedY(boundX);
			clampDimension(clippedY, bounds[2], bounds[3], drawArea.screensWidth);
			dimensions[(bitShift / 4) + index + 1] = clippedY;

			if (boundY == 0) {
				dimensions[(bitShift / 4) + index] = boundX;
			} else {
				// Specify clipped X value if cursor is also outside the top/bottom dims
				uint32_t clippedX = line.calculateClippedX(boundY);
				clampDimension(clippedX, bounds[0], bounds[1], drawArea.screensHeight);
				dimensions[(bitShift / 4) + index] = clippedX;
			}
		}
		if (boundY > 0 && boundX == 0) {
			dimensions[(bitShift / 4) + index] = line.calculateClippedX(boundY);
			dimensions[(bitShift / 4) + index + 1] = boundY;
		}
		index++;
		bitShift += 4;
	}
	return Rect(Point2d(dimensions[0], dimensions[1]), Point2d(dimensions[2], dimensions[3]));
}

void Renderer::updateRenderArea(const std::vector<Geometry*>& geometry, const Camera& camera) {
	int horizontalCount = drawArea.panels[FIRST_PERSON].getWidth() / TILE_WIDTH;
	int verticalCount = drawArea.panels[FIRST_PERSON].getHeight() / TILE_HEIGHT;

	// Use a thread to render the right hand panel and swap buffers as appropriate
	for (int i = 0; i < THREAD_COUNT; i++) {
		if (threadPool[FIRST_PERSON][i].joinable()) {
			threadPool[FIRST_PERSON][i].join();
			//threadPool[FIRST_PERSON][i] = std::thread(updateRenderAreaFunc, this, camera);
			//break;
		}
	}
	drawArea.update = true;
}

void Renderer::updateRenderArea(Renderer* instance, const Camera& camera, const int& bufferId) {
	int horizontalCount = instance->drawArea.panels[FIRST_PERSON].getWidth() / TILE_WIDTH;
	int verticalCount = instance->drawArea.panels[FIRST_PERSON].getHeight() / TILE_HEIGHT;
	Ray3d ray;
	float xCoeff, yCoeff, brightnessCoeff;
	for (float j = 0; j < verticalCount; j++) {
		for (float i = 0; i < horizontalCount; i++) {
			//ray = Ray3d(camera.x, camera.y, camera.height, camera.direction, 0);
			xCoeff = abs(i - horizontalCount / 2) / horizontalCount;
			yCoeff = abs(j - verticalCount / 2) / verticalCount;
			brightnessCoeff = sqrt((xCoeff * xCoeff + yCoeff * yCoeff) / 2);
			std::vector<uint8_t> character = instance->getCharacterBitmap(brightnessCoeff);
			instance->updateRenderArea(character, instance->drawArea.panels[FIRST_PERSON].lt.x + TILE_WIDTH * i, instance->drawArea.panels[FIRST_PERSON].lt.y + TILE_HEIGHT * j, bufferId);
		}
	}
}

void Renderer::updateRenderArea(std::vector<uint8_t> character, uint32_t x, uint32_t y, const int& bufferId) {
	for (int j = 0; j < TILE_HEIGHT; j++) {
		for (int i = 0; i < TILE_WIDTH; i++) {
			updateRenderArea(Point2d(x + i, y + j), FIRST_PERSON, (character.at(j) >> i) & 1 ? 0xff0000 : 0x0);
		}
	}
}

void Renderer::clampDimension(uint32_t& dim, const uint32_t lower, const uint32_t upper, const uint32_t screenDim) {
	if (dim < lower || dim > screenDim) {
		dim = lower;
	} else if (dim > upper && dim <= screenDim) {
		dim = upper;
	}
}