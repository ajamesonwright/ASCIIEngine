#include "geometry.h"

/*
* Take in a binary mask compare type and perform the associated comparison of geometry coordinates.
* 0b10	- compare X coordinates, s.th. return 0 indicates p1.x is left of p2.x, and return 1 indicates the opposite.
* 0b101 - compare Y coordinates, s.th. return 0 indicates p1.y is above (lower pixel value) than p2.y, and vice versa.
*/
int Geometry::comparePointsByCoordinate(CompareType compareType, const std::vector<Point2d*>* v, const Point2d* p1, const Point2d* p2, const int begin, const int end) {

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

int Geometry::comparePointVectorByCoordinate(CompareType compareType, const std::vector<Point2d*>* v, const int begin, const int end) {

	size_t range_begin, range_end;
	uint8_t compare[4] = { 0b1, 0b10, 0b100, 0b1000 };

	uint32_t compare_value = ((compare[0] & compareType) == 1) ? 0 : UINT32_MAX;
	size_t index = -1;
	uint32_t current;

	begin == -1 ? range_begin = 0 : range_begin = begin;
	end == -1 ? range_end = v->size() : range_end = end;
	// Return the index of the first instance of the highest/lowest value of the vector passed
	for (size_t i = range_begin; i < range_end; i++) {
		current = ((compare[1] & compareType) >> 1) * v->at(i)->x + ((compare[2] & compareType) >> 2) * v->at(i)->y;
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

std::vector<float> Geometry::calculateSlopes(const std::vector<Point2d*> v_g) {

	uint32_t diff_x, diff_y;
	std::vector<float> v_f;
	
	for (int i = 1; i < v_g.size(); i++) {
		diff_x = v_g[i]->x - v_g[0]->x;
		diff_y = v_g[i]->y - v_g[0]->y;
		if (diff_x == 0) {
			v_f.push_back(1.0f);
			continue;
		}
		v_f.push_back((float)diff_y / diff_x);
	}
	return v_f;
}

void Geometry::sortBySlope(std::vector<Point2d*>& vertices, const std::vector<float> slopes) {

	// sort by least negative -> most negative -> 1 -> most positive -> least positive to enforce clockwise traversal of vertices
	for (int i = 0; i < slopes.size() - 1; i++) {
		// both less than zero
		if (slopes[i] < 0 && slopes[i + 1] < 0) {
			if (slopes[i] < slopes[i + 1])
				std::swap(vertices[i + 1], vertices[i + 2]);
		}

		
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

	leftToTip = new Line(this->left, this->tip);
	rightToTip = new Line(this->right, this->tip);
	baseToTip = new Line(this->base, this->tip);
	update();
}

void Camera::update() {

	double cosx = cos(direction * M_PI / 180);
	double siny = sin(direction * M_PI / 180);
	base = Point2d((uint32_t)(x - size * cosx / 2 + 0.5), (uint32_t)(y - size * siny / 2 + 0.5));
	tip = Point2d((uint32_t)(x + size * cosx / 2 + 0.5), (uint32_t)(y + size * siny / 2 + 0.5));
	double arrow_point_length = (size / 2) * sin((fov / 2) * M_PI / 180);

	left = Point2d((uint32_t)(tip.x + (-arrow_point_length * cos((direction + (90 - fov / 2)) * M_PI / 180)) + 0.5), (uint32_t)(tip.y + (-arrow_point_length * sin((direction + (90 - fov / 2)) * M_PI / 180)) + 0.5));
	right = Point2d((uint32_t)(tip.x + (-arrow_point_length * cos((direction - (90 - fov / 2)) * M_PI / 180)) + 0.5), (uint32_t)(tip.y + (-arrow_point_length * sin((direction - (90 - fov / 2)) * M_PI / 180)) + 0.5));

}

void Camera::clampDirection() {

	if (direction < 0) {
		direction = 360.0f - (-direction - (int)(direction / -360) * 360);
		return;
	}
	direction -= (int)(direction / 360) * 360.0f;
}

void Camera::clampPosition(Point2d p, Geometry* g) {
	switch (g->type) 		{
	case (Geometry::G_LINE):
	{
	} break;
	case (Geometry::G_TRI):
	{

	} break;
	case (Geometry::G_RECT):
	{
		//if (p.x > g->vertices)

	} break;
	case (Geometry::G_CIRCLE):
	{

	} break;
	}
	
}

void Camera::clampPosition(Rect panel) {

	if ((px - size / 2) < panel.lt.x) (px = (float)panel.lt.x + size / 2);
	if ((py - size / 2) < panel.lt.y) (py = (float)panel.lt.y + size / 2);
	if ((px + size / 2) > panel.rb.x) (px = (float)panel.rb.x - size / 2);
	if ((py + size / 2) > panel.rb.y) (py = (float)panel.rb.y - size / 2);
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