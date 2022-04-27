#ifndef ASCIIENGINE_GEOMETRY_H_
#define ASCIIENGINE_GEOMETRY_H_

#include <Windows.h>
#include <stdint.h>
#include <vector>

class Point2d {

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

class Point3d : public Point2d {

public:
	uint32_t z;

	Point3d() { z = 0; };
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

class Ray2d : public Point2d {

public:
	int16_t dir; // 0-360 degrees, 0 degrees aligned with positive x-axis, positive rotation moving towards positive y-axis
	uint8_t size; // for visual representation in top down panel

	Ray2d() { x = 0; y = 0; dir = 0;  size = 10; };
	Ray2d(uint32_t p_x, uint32_t p_y, uint16_t p_dir) { x = p_x; y = p_y; dir = ClampDir(p_dir); size = 20; };

	uint16_t ClampDir(int16_t p_dir) {
		if (p_dir < 0)
			return 360 - (-p_dir - (int)(p_dir / -360) * 360);
		return p_dir - (int)(p_dir / 360) * 360;
	};
	void SetSize(uint8_t p_size) { size = p_size; };
};

struct Ray3d : public Point3d {
	
public:
	uint16_t yaw, pitch;

	Ray3d() { x = 0; y = 0; z = 0; yaw = 0; pitch = 0; };
	Ray3d(uint32_t p_x, uint32_t p_y, uint32_t p_z, uint16_t p_yaw, uint16_t p_pitch) { x = p_x; y = p_y; z = p_z; yaw = p_yaw; pitch = p_pitch; };
};

class Geometry {

public:
	void* handle; // corresponds to bottom left corner/edge
	int type;
	std::vector<Point2d*> vertices;

	Geometry() { handle = nullptr; type = -1; };

	enum geometry_type {
		G_LINE,
		G_TRI,
		G_RECT,
		G_QUAD,
		G_CIRCLE,

		G_NUM_TYPES,
	};
	
	// Collision function, must be implemented based on geometry type
	//virtual bool Collision(Point2d p) = 0;
	void SetHandle(void* p_handle) { handle = p_handle; };
protected:
	int ComparePointsByCoordinate(uint8_t compare_type, std::vector<Point2d*>* v = nullptr, Point2d* p1 = nullptr, Point2d* p2 = nullptr, int begin = -1, int end = -1);
	//void SwapPoint(Point2d& p1, Point2d& p2);
private:
	int ComparePointVectorByCoordinate(uint8_t compare_type, std::vector<Point2d*>* v, int begin = -1, int end = -1);
	int ComparePointPairByCoordinate(uint8_t compare_type, Point2d* p1, Point2d* p2);
};

class Line : public Geometry {

public:
	Point2d left, right;
	Line() {};
	Line(Point2d a, Point2d b) {
		// Using simple comparison of x and y coords for line objects
		if (a.y > b.y) {
			vertices.push_back(&a); vertices.push_back(&b);
		} else {
			vertices.push_back(&b); vertices.push_back(&a);
		}
		type = G_LINE;

		if (a.x < b.x) {
			left = a;
			right = b;
		}
		else {
			left = b;
			right = a;
		}
	};
	Line(Geometry g) {
		left = *g.vertices.at(0);
		right = *g.vertices.at(1);
	}

	Line& operator = (Line const& obj) {
		handle = obj.handle;
		type = obj.type;
		vertices = obj.vertices;
		left = obj.left;
		right = obj.right;
		return *this;
	}

	bool Collision(Point2d p) { return false; }
};

class Tri : public Geometry {

public:
	Tri() {};
	Tri(Point2d a, Point2d b, Point2d c) {
		vertices.push_back(&a); vertices.push_back(&b); vertices.push_back(&c);
		int index = ComparePointsByCoordinate(0b101, &vertices);
		if (index != 0)
			std::swap(vertices.at(0), vertices.at(index));
		handle = &vertices.at(0);
		// currently would not account for points with the same x-coord
		index = ComparePointsByCoordinate(0b10, &vertices, nullptr, nullptr, 1);
		if (index != 1)
			std::swap(*vertices.at(1), *vertices.at(index));
	};

	Tri& operator = (Tri const& obj) {
		handle = obj.handle;
		type = obj.type;
		vertices = obj.vertices;
		return *this;
	}

	bool Collision(Point2d p) { return false; }
};

// Four-sided shape that assumes orthogonal sides 
class Rect : public Geometry {

public:
	Point2d lt, rb;
	Point2d lb, rt;

	Rect() { lt = Point2d(); rb = Point2d(); lb = Point2d(); rt = Point2d(); };
	Rect(Point2d a, Point2d b) {
		lt = Point2d(min(a.x, b.x), min(a.y, b.y));
		rb = Point2d(max(a.x, b.x), max(a.y, b.y));
		lb = Point2d(min(a.x, b.x), max(a.y, b.y));
		rt = Point2d(max(a.x, b.x), min(a.y, b.y));
		vertices.push_back(&lb);
		vertices.push_back(&lt);
		vertices.push_back(&rt);
		vertices.push_back(&rb);
	};
	Rect(RECT rect) { lt = Point2d(rect.left, rect.top); rb = Point2d(rect.right, rect.bottom); lb = Point2d(rect.left, rect.bottom); rt = Point2d(rect.right, rect.top); };
	Rect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) { lt = Point2d(left, top); rb = Point2d(right, bottom); lb = Point2d(left, bottom); rt = Point2d(right, top);
	};

	Rect& operator = (Rect const& obj) {
		handle = obj.handle;
		type = obj.type;
		vertices = obj.vertices;
		lt = obj.lt;
		rb = obj.rb;
		lb = obj.lb;
		rt = obj.rt;
		return *this;
	}

	Rect& operator = (RECT const& obj) {
		lt = Point2d(obj.left, obj.top);
		rb = Point2d(obj.right, obj.bottom);
		lb = Point2d(obj.left, obj.bottom);
		rt = Point2d(obj.right, obj.top);
		handle = &lb;
		type = G_RECT;
		//vertices = malloc(type * sizeof(Point2d));
		vertices.push_back(&lb);
		vertices.push_back(&lt);
		vertices.push_back(&rt);
		vertices.push_back(&rb);
		return *this;
	}

	int GetWidth() { return this->rb.x - this->lt.x; };
	int GetHeight() { return this->rb.y - this->lt.y; };
	bool Collision(Point2d p) { return (p.x >= lt.x && p.x <= rb.x && p.y >= lt.y && p.y <= rb.y); }
};

class Quad : public Geometry {
	Point2d a, b, c, d;

	bool Collision(Point2d p) { return false; }
};

class Circle : public Geometry {
	Point2d c;
	uint16_t r = 0;

	bool Collision(Point2d p) { return c.Displacement(p) <= r; }
};

#endif