#include "geometry.h"

int Geometry::ComparePointsByCoordinate(uint8_t compare_type, std::vector<Point2d*>* v, Point2d* p1, Point2d* p2, int begin, int end) {

	// Checking for invalid compare_type values
	if (0b11110000 & compare_type || 0b1000 & compare_type || 0b0000 & compare_type)
		return -1;
	if (0b10 & compare_type && 0b100 & compare_type)
		return -1;

	if (v)
		return ComparePointVectorByCoordinate(compare_type, v, begin, end);
	if (p1 && p2)
		return ComparePointPairByCoordinate(compare_type, p1, p2);
	return -1;
}

int Geometry::ComparePointVectorByCoordinate(uint8_t compare_type, std::vector<Point2d*>* v, int begin, int end) {

	size_t range_begin, range_end;
	uint8_t compare[4] = { 0b1, 0b10, 0b100, 0b1000 };

	uint32_t compare_value = ((compare[0] & compare_type) == 1) ? 0 : UINT32_MAX;
	size_t index = -1;
	uint32_t current;

	begin == -1 ? range_begin = 0 : range_begin = begin;
	end == -1 ? range_end = v->size() : range_end = end;
	// Return the index of the first instance of the highest/lowest value of the vector passed
	for (size_t i = range_begin; i < range_end; i++) {
		Point2d p = *v->at(i);
		current = ((compare[1] & compare_type) >> 1) * p.x + ((compare[2] & compare_type) >> 2) * p.y;
		// Test the selected coordinate value against the compare value
		if (((compare[0] & compare_type) && current > compare_value) || (!(compare[0] & compare_type) && current < compare_value)) {
			compare_value = current;
			index = i;
		}
	}

	return (int)index;
}

int Geometry::ComparePointPairByCoordinate(uint8_t compare_type, Point2d* p1, Point2d* p2) {

	uint8_t compare[4] = { 0b1, 0b10, 0b100, 0b1000 };

	uint32_t compare_value[2] = { 0 };
	compare_value[0] = ((compare[1] & compare_type) >> 1) * p1->x + ((compare[2] & compare_type) >> 2) * p1->y;
	compare_value[1] = ((compare[1] & compare_type) >> 1) * p2->x + ((compare[2] & compare_type) >> 2) * p2->y;

	int index = -1;
	if (compare_value[0] == compare_value[1]) return index;

	// implement a binary int return type and use a mask to decode? may eliminate ties for priority
	if (compare[0] & compare_type) {
		(compare_value[0] > compare_value[1]) ? index = 0 : index = 1;
	} else {
		(compare_value[0] < compare_value[1]) ? index = 0 : index = 1;
	}

	return index;
}

bool Geometry::CompareBySlope(float f1, float f2) {
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

std::vector<float> Geometry::CalculateSlopes(std::vector<Point2d*> v_g) {
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

void Geometry::SortBySlope(std::vector<Point2d*>& vertices, std::vector<float> slopes) {
	// sort by least negative -> most negative -> 1 -> most positive -> least positive to enforce clockwise traversal of vertices
	for (int i = 0; i < slopes.size() - 1; i++) {
		// both less than zero
		if (slopes[i] < 0 && slopes[i + 1] < 0) {
			if (slopes[i] < slopes[i + 1])
				std::swap(vertices[i + 1], vertices[i + 2]);
		}

		
	}
}
