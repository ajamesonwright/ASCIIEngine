#ifndef ASCIIENGINE_GEOMETRY_H_
#define ASCIIENGINE_GEOMETRY_H_

#include <Windows.h>
#include <stdint.h>

struct Point {
	uint32_t x, y;

	Point() { x = 0; y = 0; };
	Point(POINT p) { x = p.x; y = p.y; };
	Point(uint32_t p_x, uint32_t p_y) { x = p_x; y = p_y; };

	Point& operator = (Point const& obj) {
		x = obj.x;
		y = obj.y;
		return *this;
	}

	Point& operator = (POINT const& obj) {
		x = obj.x;
		y = obj.y;
		return *this;
	}
};

struct Line {
	Point a, b;

	Line() { Point a; Point b; };
	Line(Point p_a, Point p_b) { a.x = p_a.x; a.y = p_a.y; b.x = p_b.x; b.y = p_b.y; };

	Line& operator = (Line const& obj) {
		a = obj.a;
		b = obj.b;
		return *this;
	}
};

struct Tri {
	Point a, b, c;

	Tri() { Point a; Point b; Point c; };
	Tri(Point p_a, Point p_b, Point p_c) { a = p_a; b = p_b; c = p_c; };

	Tri& operator = (Tri const& obj) {
		a = obj.a;
		b = obj.b;
		c = obj.c;
		return *this;
	}
};

struct Rect{
	uint32_t left, top, right, bottom;

	Rect() { left = 0; top = 0; right = 0; bottom = 0; };
	Rect(Point a, Point b) { left = min(a.x, b.x); top = min(a.y, b.y); right = max(a.x, b.x); bottom = max(a.y, b.y); };
	Rect(RECT p_rect) { left = p_rect.left; top = p_rect.top; right = p_rect.right; bottom = p_rect.bottom; };
	Rect(uint32_t p_left, uint32_t p_top, uint32_t p_right, uint32_t p_bottom) { left = p_left; top = p_top; right = p_right; bottom = p_bottom; };

	Rect& operator = (Rect const& obj) {
		left = obj.left;
		top = obj.top;
		right = obj.right;
		bottom = obj.bottom;
		return *this;
	}

	Rect& operator = (RECT const& obj) {
		left = obj.left;
		top = obj.top;
		right = obj.right;
		bottom = obj.bottom;
		return *this;
	}

	int GetWidth() { return this->right - this->left; };
	int GetHeight() { return this->bottom - this->top; };
};

struct Quad {
	Point a, b, c, d;
};

struct Circle {
	Point c;
	uint16_t r = 0;
};

struct AABB {
	Point LT, RB;

	AABB() {};
	AABB(Point p_a, Point p_b) { LT = p_a; RB = p_b; };
	AABB(Rect rect) { LT = Point(rect.left, rect.top); RB = Point(rect.right, rect.bottom); };

	AABB& operator = (Rect const& obj) {
		LT = Point(obj.left, obj.top);
		RB = Point(obj.right, obj.bottom);
		return *this;
	}

	bool Collision(Point p) {
		if (p.x >= LT.x && p.x < RB.x && p.y >= LT.y && p.y < RB.y)
			return true;
		return false;
	}
};

#endif