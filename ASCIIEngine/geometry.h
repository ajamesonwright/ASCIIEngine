#ifndef ASCIIENGINE_GEOMETRY_H_
#define ASCIIENGINE_GEOMETRY_H_

#include <Windows.h>
#include <stdint.h>

struct Point {
	uint32_t x, y;

	Point() { x = 0; y = 0; };
	Point(uint32_t p_x, uint32_t p_y) { x = p_x; y = p_y; };

	Point operator = (Point const& obj) {
		Point result;
		result.x = obj.x;
		result.y = obj.y;
		return result;
	}
};

struct Line {
	Point a, b;

	Line() { Point a; Point b; };
	Line(Point p_a, Point p_b) { a = p_a; b = p_b; };

	Line operator = (Line const& obj) {
		Line result;
		result.a = obj.a;
		result.b = obj.b;
		return result;
	}
};

struct Tri {
	Point a, b, c;

	Tri() { Point a; Point b; Point c; };
	Tri(Point p_a, Point p_b, Point p_c) { a = p_a; b = p_b; c = p_c; };

	Tri operator = (Tri const& obj) {
		Tri result;
		result.a = obj.a;
		result.b = obj.b;
		result.c = obj.c;
		return result;
	}
};

struct Rect{
	uint32_t left, top, right, bottom;

	Rect() { left = 0; top = 0; right = 0; bottom = 0; };
	Rect(RECT p_rect) { left = p_rect.left; top = p_rect.top; right = p_rect.right; bottom = p_rect.bottom; };
	Rect(uint32_t p_left, uint32_t p_top, uint32_t p_right, uint32_t p_bottom) { left = p_left; top = p_top; right = p_right; bottom = p_bottom; };

	Rect operator = (Rect const& obj) {
		Rect result;
		result.left = obj.left;
		result.top = obj.top;
		result.right = obj.right;
		result.bottom = obj.bottom;
		return result;
	}
};

struct Quad {
	Point a, b, c, d;
};

struct Circle {
	Point c;
	uint16_t r = 0;
};

#endif