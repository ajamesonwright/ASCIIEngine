#pragma once
#include <vector>

#include "Geometry.h"

using std::vector;

// Unused for now, as the geometry methods work. This could be implemented using function pointers in the Geometry class to reduce generalization of the compare functions
//int (*ComparePointsByX) (Point2d* p1, Point2d* p2);
//int (*ComparePointsByY) (Point2d* p1, Point2d* p2);
//int (*ComparePointVector) (vector<Point2d*>, int begin, int end);

int comparePointsByX(Point2d* p1, Point2d* p2) {

}
int comparePointsByY(Point2d* p1, Point2d* p2) {

}