#include "geometry.h"

void Geometry::createVertexShell() {
	for (int i = 0; i < vertices.size(); i++) {
		sides.push_back(Line(vertices.at(i), vertices.at(i + 1 == vertices.size() ? 0 : i + 1)));
	}
}

/*
* Take in a binary mask compare type and perform the associated comparison of geometry coordinates.
* Returns -1 indicating that the coordinate specified in the compare type for the first point is the same as that of the second
* Returns 0 indicating that the coordinate specified in the compare type for the first point is less than that of the second.
* Returns 1 indicating the opposite.
*/
int Geometry::comparePointsByCoordinate(CompareType compareType, const std::vector<Point2d>* v, const Point2d* p1, const Point2d* p2, const int begin, const int end) {

	// Checking for invalid compare_type values
	if (CompareType::UNDEFINED_COMPARE & compareType) {
		throw "Invalid compare type!";
		return -1;
	}
	if (CompareType::COMPARE_BY_X & compareType && CompareType::COMPARE_BY_Y & compareType) {
		throw "Invalid compare type!";
		return -1;
	}

	if (v)
		return comparePointVectorByCoordinate(compareType, v, begin, end);
	if (p1 && p2)
		return comparePointPairByCoordinate(compareType, p1, p2);
	return -1;
}

int Geometry::comparePointVectorByCoordinate(CompareType compareType, const std::vector<Point2d>* v, const int begin, const int end) {

	size_t range_begin, range_end;
	uint8_t compare[4] = { 0b1, 0b10, 0b100, 0b1000 };

	uint32_t compare_value = ((compare[0] & compareType) == 1) ? 0 : UINT32_MAX;
	size_t index = -1;
	uint32_t current;

	begin == -1 ? range_begin = 0 : range_begin = begin;
	end == -1 ? range_end = v->size() : range_end = end;
	// Return the index of the first instance of the highest/lowest value of the vector passed
	for (size_t i = range_begin; i < range_end; i++) {
		current = ((compare[1] & compareType) >> 1) * v->at(i).x + ((compare[2] & compareType) >> 2) * v->at(i).y;
		// Test the selected coordinate value against the compare value
		if (((compare[0] & compareType) && current > compare_value) || (!(compare[0] & compareType) && current < compare_value)) {
			compare_value = current;
			index = i;
		}
	}

	return (int)index;
}

int Geometry::comparePointPairByCoordinate(CompareType compareType, const Point2d* p1, const Point2d* p2) {

	// Compare ->                 X     Y
	uint8_t compare[4] = { 0b1, 0b10, 0b100, 0b1000 };

	uint32_t compare_value[2] = { 0 };
	// Filter out the coordinate we don't want
	compare_value[0] = ((compare[1] & compareType) >> 1) * p1->x + ((compare[2] & compareType) >> 2) * p1->y;
	compare_value[1] = ((compare[1] & compareType) >> 1) * p2->x + ((compare[2] & compareType) >> 2) * p2->y;

	int index = -1;
	if (compare_value[0] == compare_value[1]) return index;

	// implement a binary int return type and use a mask to decode? may eliminate ties for priority
	if (compare[0] & compareType) {
		(compare_value[0] > compare_value[1]) ? index = 0 : index = 1;
	} else {
		(compare_value[0] < compare_value[1]) ? index = 0 : index = 1;
	}

	return index;
}

uint8_t Geometry::comparePointPairByCoordinates(const Point2d* p1, const Point2d* p2) {
	// Compare by x-value, then by y-value

	// Return an 8-bit integer containing comparisons of the coordinates of each point
	uint8_t result = 0b0;

	result |= (COMPARE_BY_X & (int)(p1->x < p2->x)) << 0;
	result |= (COMPARE_BY_Y & (int)(p1->y < p2->y)) << 1;

	return result;
}

bool Geometry::compareBySlope(const float f1, float f2) {

	// return indicates if floats are in their correct positions in vector
	if (f1 < 0) {
		if (f2 < 0)
			return f1 > f2;
		else if (f2 > 0)
			return true;
		return true;

	}

	return false;
}

std::vector<float> Geometry::calculateSlopes(const std::vector<Point2d> v_g) {

	uint32_t diff_x, diff_y;
	std::vector<float> v_f;
	
	for (int i = 1; i < v_g.size(); i++) {
		diff_x = v_g[i].x - v_g[0].x;
		diff_y = v_g[i].y - v_g[0].y;
		if (diff_x == 0) {
			v_f.push_back(1.0f);
			continue;
		}
		v_f.push_back((float)diff_y / diff_x);
	}
	return v_f;
}

void Geometry::sortBySlope(std::vector<Point2d>& vertices, const std::vector<float> slopes) {

	// sort by least negative -> most negative -> 1 -> most positive -> least positive to enforce clockwise traversal of vertices
	for (int i = 0; i < slopes.size() - 1; i++) {
		// both less than zero
		if (slopes[i] < 0 && slopes[i + 1] < 0) {
			if (slopes[i] < slopes[i + 1])
				std::swap(vertices[i + 1], vertices[i + 2]);
		}
	}
}

Line::Line(const Line& source) {
	type = source.type;
	vertices.clear();
	vertices.push_back(source.vertices[0]);
	vertices.push_back(source.vertices[1]);
	initialized = true;
}

Line::Line(const Geometry& source) {
	if (source.type != G_LINE || source.vertices.size() != 2)
		return;
	type = source.type;
	vertices.clear();
	vertices.push_back(source.vertices[0]);
	vertices.push_back(source.vertices[1]);
	initialized = true;
};

Line::Line(const Point2d& a, const Point2d& b) : Geometry(G_LINE) {
	vertices.clear();
	vertices.push_back(a);
	vertices.push_back(b);
	initialized = true;
	return;
}

bool Line::checkCollisionWith(const Point2d& p) {
	return calculateClippedY(p.x) == p.y;
}

void Line::checkCollisionWith(Geometry* g, std::vector<Point2d>& collisions, std::vector<Line>& interferingSides) {
	switch (g->type) {
	case (Geometry::G_LINE):
	{
		Line l = static_cast<Line>(*g);
		findIntersection(l, collisions, interferingSides);
	} break;
	case (Geometry::G_TRI):
	{
		Tri t = static_cast<Tri>(*g);
		Line sides[3] = { Line(t.vertices.at(0), t.vertices.at(1)), Line(t.vertices.at(1), t.vertices.at(2)), Line(t.vertices.at(2), t.vertices.at(0)) };
		for (Line l : sides) {
			findIntersection(l, collisions, interferingSides);
		}
	} break;
	case (Geometry::G_RECT):
	{
		Rect r = static_cast<Rect>(*g);
		checkCollisionWith(r, collisions, interferingSides);
	} break;
	case (Geometry::G_QUAD):
	{
		Quad q = static_cast<Quad>(*g);
		Line sides[4] = { Line(q.vertices.at(0), q.vertices.at(1)), Line(q.vertices.at(1), q.vertices.at(2)), Line(q.vertices.at(2), q.vertices.at(3)), Line(q.vertices.at(3), q.vertices.at(0)) };
		for (Line l : sides) {
			findIntersection(l, collisions, interferingSides);
		}
	} break;
	case (Geometry::G_CIRCLE):
	{

	} break;
	}
}

void Line::checkCollisionWith(Rect r, std::vector<Point2d>& collisions, std::vector<Line>& interferingSides) {
	Line sides[4] = { Line(r.lb, r.lt), Line(r.lt, r.rt), Line(r.rt, r.rb), Line(r.rb, r.lb) };
	for (Line l : sides) {
		findIntersection(l, collisions, interferingSides);
	}
}

void Line::checkCollisionWith(Camera* c, std::vector<Point2d>& collisions, std::vector<Line>& interferingSides) {
	Line sides[4] = { c->leftSide, c->front, c->rightSide, c->back };
	for (Line l : sides) {
		findIntersection(l, collisions, interferingSides);
	}
}

void Line::findIntersection(const Line& l, std::vector<Point2d>& collisions, std::vector<Line>& interferingSides) {
	Point2d intersection;
	findIntersection(l, intersection);
	if (intersection.isInitialized()) {
		collisions.push_back(intersection);
		interferingSides.push_back(l);
	}
}

Point2d Line::findClosestPointOnLine(const Point2d& p) {
	Line l = Line(vertices.at(0), p);
	double ptox1 = (double)p.x - vertices.at(0).x;
	double ptoy1 = (double)p.y - vertices.at(0).y;

	double dotProduct = ptox1 * getDx() + ptoy1 * getDy();
	double lengthSquared = getDy() * getDy() + getDx() * getDx();
	double param = (lengthSquared != 0) ? (dotProduct / lengthSquared) : -1;

	Point2d closestPoint;
	if (param < 0) {
		closestPoint = vertices.at(0);
	} else if (param > 1) {
		closestPoint = vertices.at(1);
	} else {
		closestPoint.x = vertices.at(0).x + param * getDx();
		closestPoint.y = vertices.at(0).y + param * getDy();
	}

	return closestPoint;
}

double Line::calculateIntercept() {
	return vertices.at(0).y - calculateSlope() * vertices.at(0).x;
}

double Line::calculateLength() {
	return sqrt(getDx() * getDx() + getDy() * getDy());
}

double Line::calculateAngle(Line& l) {
	// Produces a value in the range (0,180]. If the value of the swept angle from positive to positive is >=180, val = 360 - val
	return acos((getDx() * l.getDx() + getDy() * l.getDy()) / (calculateLength() * l.calculateLength()));
}

double Line::calculateDotProduct(Line& l1, Line& l2) {
	return l1.getDx() * l2.getDx() + l1.getDy() * l2.getDy();
}

uint32_t Line::calculateClippedY(uint32_t bound) {
	double m = calculateSlope();
	double b = calculateIntercept();
	double D = m * bound + b;
	uint32_t d = static_cast<uint32_t>(D);
	
	if (D - d > 0.5) {
		return d + 1;
	}
	return d;
}

uint32_t Line::calculateClippedX(uint32_t bound) {
	// Special case for vertical line
	if (getDx() == 0) {
		return vertices.at(0).x;
	}
	double m = calculateSlope();
	double b = calculateIntercept();
	double D = (bound - b) / m;
	uint32_t d = static_cast<uint32_t>(D);

	if (D - d > 0.5) {
		return d + 1;
	}
	return d;
}

bool Line::hasOverlappingDomain(Line l) {
	// Calculate the range of x values commons to both lines
	uint32_t minX = min(vertices.at(0).x, vertices.at(1).x);
	uint32_t maxX = max(vertices.at(0).x, vertices.at(1).x);
	uint32_t minLX = min(l.vertices.at(0).x, l.vertices.at(1).x);
	uint32_t maxLX = max(l.vertices.at(0).x, l.vertices.at(1).x);
	return (minX >= minLX && minX <= maxLX
		|| maxX >= minLX && maxX <= maxLX
		|| minLX >= minX && minLX <= maxX
		|| maxLX >= minX && maxLX <= maxX);
}

bool Line::hasOverlappingRange(Line l) {
	// Calculate the range of y values commons to both lines
	uint32_t minY = min(vertices.at(0).y, vertices.at(1).y);
	uint32_t maxY = max(vertices.at(0).y, vertices.at(1).y);
	uint32_t minLY = min(l.vertices.at(0).y, l.vertices.at(1).y);
	uint32_t maxLY = max(l.vertices.at(0).y, l.vertices.at(1).y);
	return (minY >= minLY && minY <= maxLY
		|| maxY >= minLY && maxY <= maxLY
		|| minLY >= minY && minLY <= maxY
		|| maxLY >= minY && maxLY <= maxY);
}

void Line::findIntersection(Line l, Point2d& intersection) {
	
	// Line AB represented as a1x + b1y = c1
	double a1 = static_cast<double>(l.vertices.at(1).y) - l.vertices.at(0).y; // cast to double to prevent unsigned calculation from rolling over
	double b1 = static_cast<double>(l.vertices.at(0).x) - l.vertices.at(1).x;
	double c1 = a1 * (l.vertices.at(0).x) + b1 * (l.vertices.at(0).y);

	// Line CD represented as a2x + b2y = c2
	double a2 = static_cast<double>(vertices.at(1).y) - vertices.at(0).y;
	double b2 = static_cast<double>(vertices.at(0).x) - vertices.at(1).x;
	double c2 = a2 * (vertices.at(0).x) + b2 * (vertices.at(0).y);

	double determinant = a1 * b2 - a2 * b1;

	if (determinant != 0) {
		uint32_t minX = min(vertices.at(0).x, vertices.at(1).x);
		uint32_t maxX = max(vertices.at(0).x, vertices.at(1).x);
		uint32_t minY = min(vertices.at(0).y, vertices.at(1).y);
		uint32_t maxY = max(vertices.at(0).y, vertices.at(1).y);
		uint32_t minLX = min(l.vertices.at(0).x, l.vertices.at(1).x);
		uint32_t maxLX = max(l.vertices.at(0).x, l.vertices.at(1).x);
		uint32_t minLY = min(l.vertices.at(0).y, l.vertices.at(1).y);
		uint32_t maxLY = max(l.vertices.at(0).y, l.vertices.at(1).y);

		uint32_t x = static_cast<uint32_t>((b2 * c1 - b1 * c2) / determinant + 0.5);
		uint32_t y = static_cast<uint32_t>((a1 * c2 - a2 * c1) / determinant + 0.5);

		if (x >= minX && x <= maxX && y >= minY && y <= maxY &&
			x >= minLX && x <= maxLX && y >= minLY && y <= maxLY) {
			intersection = Point2d(x, y);
		}
	}
}

Tri::Tri(const Tri& source) : Geometry(G_TRI) {
	type = source.type;
	vertices.clear();
	for (int i = 0; i < source.vertices.size(); i++) {
		vertices.push_back(source.vertices[i]);
	}
}

Tri::Tri(const Geometry& source) {
	if (source.type != G_TRI || source.vertices.size() != 3)
		return;

	type = source.type;
	vertices.clear();
	for (int i = 0; i < source.vertices.size(); i++) {
		vertices.push_back(source.vertices[i]);
	}
};

Tri::Tri(const Point2d& a, const Point2d& b, const Point2d& c) : Geometry(G_TRI) {
	vertices.clear();
	vertices.push_back(a); vertices.push_back(b); vertices.push_back(c);
	int index = comparePointsByCoordinate(CompareType::COMPARE_BY_Y, &vertices);
	if (index != 0)
		std::swap(vertices.at(0), vertices.at(index));
	// currently would not account for points with the same x-coord (TODO: implement binary int return type and masking for decode)
	index = comparePointsByCoordinate(CompareType::COMPARE_BY_X, &vertices, nullptr, nullptr, 1);
	if (index != 1)
		std::swap(vertices.at(1), vertices.at(index));
};

void Tri::checkCollisionWith(Geometry* g, std::vector<Point2d>& collisions, std::vector<Line>& interferingSides) {
	switch (g->type) {
	case (Geometry::G_LINE):
	{
	} break;
	case (Geometry::G_TRI):
	{

	} break;
	case (Geometry::G_RECT):
	{

	} break;
	case (Geometry::G_QUAD):
	{

	} break;
	case (Geometry::G_CIRCLE):
	{

	} break;
	}
}

Rect::Rect(const Rect& source) {
	if (source.vertices.size() != 4)
		return;

	type = source.type;
	vertices.clear();
	for (int i = 0; i < source.vertices.size(); i++) {
		vertices.push_back(source.vertices[i]);
	}
	lt = source.lt;
	rb = source.rb;
	lb = source.lb;
	rt = source.rt;
}

Rect::Rect(const Geometry& source) {
	if (source.type != G_RECT || source.vertices.size() != 4)
		return;

	type = source.type;
	vertices.clear();
	for (int i = 0; i < source.vertices.size(); i++) {
		vertices.push_back(source.vertices[i]);
	}
	lb = vertices.at(0); // Ordering of pairs is enforced from creation by points, therefore existing pattern can be used to determine order instead of testing
	lt = vertices.at(1);
	rt = vertices.at(2);
	rb = vertices.at(3);
};

Rect::Rect(const Point2d& a, const Point2d& b) : Geometry(G_RECT) {
	lb = Point2d(min(a.x, b.x), max(a.y, b.y));
	lt = Point2d(min(a.x, b.x), min(a.y, b.y));
	rt = Point2d(max(a.x, b.x), min(a.y, b.y));
	rb = Point2d(max(a.x, b.x), max(a.y, b.y));
	vertices.clear();
	vertices.push_back(lb);
	vertices.push_back(lt);
	vertices.push_back(rt);
	vertices.push_back(rb);
};

Rect::Rect(const RECT& rect) : Geometry(G_RECT) {
	lb = Point2d(rect.left, rect.bottom);
	lt = Point2d(rect.left, rect.top);
	rt = Point2d(rect.right, rect.top);
	rb = Point2d(rect.right, rect.bottom);
	vertices.clear();
	vertices.push_back(lb);
	vertices.push_back(lt);
	vertices.push_back(rt);
	vertices.push_back(rb);
};


void Rect::checkCollisionWith(Geometry* g, std::vector<Point2d>& collisions, std::vector<Line>& interferingSides) {
	switch (g->type) {
	case (Geometry::G_LINE):
	{
		Line l = static_cast<Line>(*g);
		// Needs to account for collisions with all sides of rect
		uint32_t leftSideY = l.calculateClippedY(lt.x);
		uint32_t rightSideY = l.calculateClippedY(rt.x);
		bool lineCrossesSides = leftSideY || rightSideY;

		if (leftSideY) {
			Point2d p = Point2d(lt.x, leftSideY);
			collisions.push_back(p);
		}
		if (rightSideY) {
			Point2d p = Point2d(rt.x, rightSideY);
			collisions.push_back(p);
		}
	} break;
	case (Geometry::G_TRI):
	{

	} break;
	case (Geometry::G_RECT):
	{

	} break;
	case (Geometry::G_QUAD):
	{

	} break;
	case (Geometry::G_CIRCLE):
	{

	} break;
	}
}

Quad::Quad(const Quad& source) {
	if (source.type != G_QUAD || source.vertices.size() != 4)
		return;

	type = source.type;
	vertices.clear();
	for (int i = 0; i < source.vertices.size(); i++) {
		vertices.push_back(source.vertices[i]);
	}
};

Quad::Quad(const Geometry& source) {
	if (source.type != G_QUAD || source.vertices.size() != 4)
		return;

	type = source.type;
	vertices.clear();
	for (int i = 0; i < source.vertices.size(); i++) {
		vertices.push_back(source.vertices[i]);
	}
}
void Quad::checkCollisionWith(Geometry* g, std::vector<Point2d>& collisions, std::vector<Line>& interferingSides) {
	switch (g->type) {
	case (Geometry::G_LINE):
	{
	} break;
	case (Geometry::G_TRI):
	{

	} break;
	case (Geometry::G_RECT):
	{

	} break;
	case (Geometry::G_QUAD):
	{

	} break;
	case (Geometry::G_CIRCLE):
	{

	} break;
	}
}

Circle::Circle(const Point2d& p_c, uint16_t p_r) : Geometry(G_CIRCLE) {
	center = p_c;
	r = p_r;
	vertices.push_back(center);
	createVertexShell();
}

Circle::Circle(const Point2d& p_c, const Point2d& p_r) : Geometry(G_CIRCLE) {
	center = p_c;
	r = static_cast<uint16_t>(sqrt(((p_c.x - p_r.x) * (p_c.x - p_r.x)) + ((p_c.y - p_r.y) * (p_c.y - p_r.y))));
	vertices.push_back(center);
	createVertexShell();
}

Circle::Circle(const Circle& source) : Geometry(G_CIRCLE) {
	center = source.center;
	r = source.r;
	vertices.push_back(center);
	for (Point2d v : source.vertices) {
		vertices.push_back(v);
	}
}

Circle::Circle(const Geometry& source) {
	if (source.type != G_CIRCLE && source.vertices.size() != Circle::SIDE_COUNT + 1) {
		return;
	}
	
	type = source.type;
	for (Point2d v : source.vertices) {
		vertices.push_back(v);
	}
	center = source.vertices.at(0);
}

void Circle::checkCollisionWith(Geometry* g, std::vector<Point2d>& collisions, std::vector<Line>& interferingSides) {
	switch (g->type) {
	case (Geometry::G_LINE):
	{
	} break;
	case (Geometry::G_TRI):
	{

	} break;
	case (Geometry::G_RECT):
	{

	} break;
	case (Geometry::G_QUAD):
	{

	} break;
	case (Geometry::G_CIRCLE):
	{

	} break;
	}
}

void Circle::createVertexShell() {
	double increment = 360 / Circle::SIDE_COUNT;
	for (int i = 0; i < Circle::SIDE_COUNT; i++) {
		double cosx = cos(i * increment * M_PI / 180);
		double siny = sin(i * increment * M_PI / 180);
		Point2d v = Point2d(center.x + static_cast<int32_t>(r * cosx), center.y + static_cast<int32_t>(r * siny));
		vertices.push_back(v);
	}
}


Camera::Camera(uint32_t p_x, uint32_t p_y, float p_direction) {
	x = p_x; y = p_y;
	direction = p_direction;
	clampDirection();
	size = 10;
	px = (float)p_x; py = (float)p_y;
	vx = 0.0f; vy = 0.0f;
	ax = 0.0f; ay = 0.0f;

	update();
}

void Camera::update() {
	// Direction = 0 - 360, clockwise from positive x axis
	double cosx = cos(direction * M_PI / 180);
	double siny = sin(direction * M_PI / 180);
	base = Point2d((uint32_t)(x - size * cosx / 2 + 0.5), (uint32_t)(y - size * siny / 2 + 0.5));
	tip = Point2d((uint32_t)(x + size * cosx / 2 + 0.5), (uint32_t)(y + size * siny / 2 + 0.5));
	double arrow_point_length = (size / 2) * sin((fov / 2) * M_PI / 180);

	left = Point2d((uint32_t)(tip.x + (-arrow_point_length * cos((direction + (90 - fov / 2)) * M_PI / 180)) + 0.5), (uint32_t)(tip.y + (-arrow_point_length * sin((direction + (90 - fov / 2)) * M_PI / 180)) + 0.5));
	right = Point2d((uint32_t)(tip.x + (-arrow_point_length * cos((direction - (90 - fov / 2)) * M_PI / 180)) + 0.5), (uint32_t)(tip.y + (-arrow_point_length * sin((direction - (90 - fov / 2)) * M_PI / 180)) + 0.5));

	// Maintain bounding box for collisions
	boundingBox[0] = Point2d(static_cast<uint32_t>(x + c2C * cos((direction - (90 + theta)) * M_PI / 180) + 0.5), static_cast<uint32_t>(y + c2C * sin((direction - (90 + theta)) * M_PI / 180) + 0.5));
	boundingBox[1] = Point2d(static_cast<uint32_t>(x + c2C * cos((direction - (90 - theta)) * M_PI / 180) + 0.5), static_cast<uint32_t>(y + c2C * sin((direction - (90 - theta)) * M_PI / 180) + 0.5));
	boundingBox[2] = Point2d(static_cast<uint32_t>(x + c2C * cos((direction + (90 - theta)) * M_PI / 180) + 0.5), static_cast<uint32_t>(y + c2C * sin((direction + (90 - theta)) * M_PI / 180) + 0.5));
	boundingBox[3] = Point2d(static_cast<uint32_t>(x + c2C * cos((direction + (90 + theta)) * M_PI / 180) + 0.5), static_cast<uint32_t>(y + c2C * sin((direction + (90 + theta)) * M_PI / 180) + 0.5));
	leftSide = Line(boundingBox[0], boundingBox[1]);
	front = Line(boundingBox[1], boundingBox[2]);
	rightSide = Line(boundingBox[2], boundingBox[3]);
	back = Line(boundingBox[3], boundingBox[0]);

}

void Camera::clampDirection() {
	if (direction < 0) {
		direction = 360.0f - (-direction - (int)(direction / -360) * 360);
		return;
	}
	direction -= (int)(direction / 360) * 360.0f;
}

void Camera::clampPosition(Rect panel) {
	if ((px - size / 2) < panel.lt.x) (px = (float)panel.lt.x + size / 2);
	if ((py - size / 2) < panel.lt.y) (py = (float)panel.lt.y + size / 2);
	if ((px + size / 2) > panel.rb.x) (px = (float)panel.rb.x - size / 2);
	if ((py + size / 2) > panel.rb.y) (py = (float)panel.rb.y - size / 2);
}

void Camera::clampPosition(const Line& l, const Point2d collision) {
	Point2d center = Point2d(x, y);
	Line normal = Line(collision, center);
	Line xAxis = Line(Point2d(0, 0), Point2d(1, 0));
	float alpha = xAxis.calculateAngle(normal);
	if (normal.calculateLength() < (size / 2) + 2) {
		px = collision.x + (size / 2 + 2) * (normal.getDx() == 0 ? 0 : normal.getDx() / abs(normal.getDx())); // should include something that corrects for direction
		py = collision.y + (size / 2 + 2) * (normal.getDy() == 0 ? 0 : normal.getDy() / abs(normal.getDy()));
	}
	
	x = (uint32_t)(px);
	y = (uint32_t)(py);
}

void Camera::clampVelocity() {
	float limit = 100.0f;
	if (vx > limit)
		vx = limit;
	if (vy > limit)
		vy = limit;
	if (vx < -limit)
		vx = -limit;
	if (vy < -limit)
		vy = -limit;
}
void Camera::clampAcceleration() {
	float limit = 2000.0f;
	if (ax > limit) ax = limit;
	if (ay > limit) ay = limit;
	if (ax < -limit) ax = -limit;
	if (ay < -limit) ay = -limit;
}

void Camera::clampAngularAcceleration() {
	float limit = 4000.0f;
	if (aa > limit) aa = limit;
	if (aa < -limit) aa = -limit;
}

void Camera::checkCollisionWith(Geometry* g, std::map<Point2d, std::vector<Line>>& collisions) {
	switch (g->type) {
	case (Geometry::G_LINE):
	{
		Line l = static_cast<Line>(*g);
		findIntersection(l, collisions);
	} break;
	case (Geometry::G_TRI):
	{
		Tri t = static_cast<Tri>(*g);
		Line sides[3] = { Line(t.vertices.at(0), t.vertices.at(1)), Line(t.vertices.at(1), t.vertices.at(2)), Line(t.vertices.at(2), t.vertices.at(0)) };
		for (Line l : sides) {
			findIntersection(l, collisions);
		}
	} break;
	case (Geometry::G_RECT):
	{
		Rect r = static_cast<Rect>(*g);
		Line sides[4] = { Line(r.lb, r.lt), Line(r.lt, r.rt), Line(r.rt, r.rb), Line(r.rb, r.lb) };
		for (Line l : sides) {
			findIntersection(l, collisions);
		}
	} break;
	case (Geometry::G_QUAD):
	{
		Quad q = static_cast<Quad>(*g);
		Line sides[4] = { Line(q.vertices.at(0), q.vertices.at(1)), Line(q.vertices.at(1), q.vertices.at(2)), Line(q.vertices.at(2), q.vertices.at(3)), Line(q.vertices.at(3), q.vertices.at(0)) };
		for (Line l : sides) {
			findIntersection(l, collisions);
		}
	} break;
	case (Geometry::G_CIRCLE):
	{
		Circle c = static_cast<Circle>(*g);

		Line sides[Circle::SIDE_COUNT];
		for (int i = 1; i < Circle::SIDE_COUNT; i++) {
			sides[i] = Line(c.vertices.at(i), c.vertices.at(i + 1 == Circle::SIDE_COUNT ? 1 : i + 1));
		}
		for (Line l : sides) {
			findIntersection(l, collisions);
		}
	} break;
	}
}

void Camera::findIntersection(Line& l, std::map<Point2d, std::vector<Line>>& collisions) {
	Point2d center = Point2d(x, y);
	Point2d p = l.findClosestPointOnLine(center);
	if (p.displacementFrom(center) <= (size / 2)) {
		auto it = collisions.find(p);

		if (it != collisions.end()) {
			it->second.push_back(l);
		} else {
			std::vector<Line> interferingSides = { l };
			collisions.insert({ p, interferingSides });
		}
	}
}
