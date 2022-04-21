#ifndef ASCIIENGINE_GEOMETRY_H_
#define ASCIIENGINE_GEOMETRY_H_

#include <Windows.h>
#include <stdint.h>
#include <vector>

struct Point2d {

public:
	uint32_t x, y;

	Point2d() { x = 0; y = 0; };
	Point2d(POINT p) { x = p.x; y = p.y; };
	Point2d(uint32_t p_x, uint32_t p_y) { x = p_x; y = p_y; };

	Point2d& operator = (Point2d const& obj) {
		x = obj.x;
		y = obj.y;
		return *this;
	}

	Point2d& operator = (POINT const& obj) {
		x = obj.x;
		y = obj.y;
		return *this;
	}

	bool operator == (Point2d const& obj) {
		return x == obj.x && y == obj.y;
	}

	int Displacement(Point2d p) { return (int)sqrt(abs((int)(x - p.x)) * abs((int)(x - p.x)) + abs((int)(y - p.y)) * abs((int)(y - p.y))); };
};

struct Point3d {

public:
	uint32_t x, y, z;

	Point3d() { x = 0; y = 0; z = 0; };
	Point3d(Point2d p, uint32_t p_z) { x = p.x; y = p.y; z = p_z; };

	Point3d& operator = (Point3d const& obj) {
		x = obj.x;
		y = obj.y;
		z = obj.z;
		return *this;
	}

	bool operator == (Point3d const& obj) {
		return x == obj.x && y == obj.y && z == obj.z;
	}
};

struct Ray2d : public Point2d {

public:
	uint16_t dir;

	Ray2d() { x = 0; y = 0; dir = 0; };
	Ray2d(uint32_t p_x, uint32_t p_y, uint16_t p_dir) { x = p_x; y = p_y; dir = p_dir; };
};

struct Ray3d : public Point3d {

};

class Geometry {

public:
	struct GeometryData {
		void* handle;
		int type;
		std::vector<Point2d> vertices;
	};

	enum geometry_type {
		G_LINE,
		G_TRI,
		G_RECT,
		G_QUAD,
		G_CIRCLE,
		G_AABB,

		G_NUM_TYPES,
	};
	
	// Collision function, must be implemented based on geometry type
	virtual bool Collision(Point2d p) = 0;
};

class Line : public Geometry {

public:
	Point2d a, b;

	Line() { Point2d a; Point2d b; };
	Line(Point2d p_a, Point2d p_b) { a.x = p_a.x; a.y = p_a.y; b.x = p_b.x; b.y = p_b.y; };

	Line& operator = (Line const& obj) {
		a = obj.a;
		b = obj.b;
		return *this;
	}

	bool Collision(Point2d p) override { return false; }
};

class Tri : public Geometry {

public:
	Point2d a, b, c;

	Tri() { Point2d a; Point2d b; Point2d c; };
	Tri(Point2d p_a, Point2d p_b, Point2d p_c) { a = p_a; b = p_b; c = p_c; };

	Tri& operator = (Tri const& obj) {
		a = obj.a;
		b = obj.b;
		c = obj.c;
		return *this;
	}

	bool Collision(Point2d p) override { return false; }
};

// Four-sided shape that assumes orthogonal sides 
class Rect : public Geometry {

public:
	Point2d LT, RB;
	//uint32_t left, top, right, bottom;

	Rect() { LT = Point2d(); RB = Point2d(); };
	Rect(Point2d a, Point2d b) { LT = Point2d(min(a.x, b.x), min(a.y, b.y)); RB = Point2d(max(a.x, b.x), max(a.y, b.y)); }; // does not assume that a is LT and b is RB
	Rect(RECT rect) { LT = Point2d(rect.left, rect.top); RB = Point2d(rect.right, rect.bottom); };
	Rect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) { LT = Point2d(left, top); RB = Point2d(right, bottom); };

	Rect& operator = (Rect const& obj) {
		LT = obj.LT;
		RB = obj.RB;
		return *this;
	}

	Rect& operator = (RECT const& obj) {
		LT = Point2d(obj.left, obj.top);
		RB = Point2d(obj.right, obj.bottom);
		return *this;
	}

	int GetWidth() { return this->RB.x - this->LT.x; };
	int GetHeight() { return this->RB.y - this->LT.y; };
	bool Collision(Point2d p) override { return (p.x >= LT.x && p.x < RB.x&& p.y >= LT.y && p.y < RB.y); }
};

class Quad : public Geometry {
	Point2d a, b, c, d;

	bool Collision(Point2d p) override { return false; }
};

class Circle : public Geometry {
	Point2d c;
	uint16_t r = 0;

	bool Collision(Point2d p) override { return false; }
};

class AABB : public Geometry {

public:
	Point2d LT, RB;

	//AABB() {};
	//AABB(Point p_a, Point p_b) { LT = p_a; RB = p_b; };
	//AABB(Rect rect) { LT = Point(rect.left, rect.top); RB = Point(rect.right, rect.bottom); };

	//AABB& operator = (Rect const& obj) {
	//	LT = Point(obj.left, obj.top);
	//	RB = Point(obj.right, obj.bottom);
	//	return *this;
	//}

	//bool Collision(Point p) {
	//	if (p.x >= LT.x && p.x < RB.x && p.y >= LT.y && p.y < RB.y)
	//		return true;
	//	return false;
	//}
};

#endif