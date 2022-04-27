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

	uint32_t compare_value = (compare[0] & compare_type) ? UINT32_MAX : 0;
	size_t index = -1;
	uint32_t current;

	begin == -1 ? range_begin = 0 : range_begin = begin;
	end == -1 ? range_end = v->size() : range_end = end;
	for (size_t i = range_begin; i < range_end; i++) {
		Point2d p = *v->at(i);
		current = (compare_type >> 1) * p.x + (compare_type >> 2) * p.y;
		// Test teh selected coordinate value against the compare value
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
	compare_value[0] = (compare[1] & compare_type) * p1->x + (compare[2] & compare_type) * p1->y;
	compare_value[1] = (compare[1] & compare_type) * p2->x + (compare[2] & compare_type) * p2->y;

	int index = -1;
	if (compare[0] & compare_type) {
		(compare_value[0] >= compare_value[1]) ? index = 0 : index = 1;
	} else {
		(compare_value[0] <= compare_value[1]) ? index = 0 : index = 1;
	}

	return index;
}

//void Geometry::SwapPoint(Point2d& p1, Point2d& p2) {
//	Point2d* temp;
//	temp = &p1;
//	p1 = p2;
//	p2 = *temp;
//}