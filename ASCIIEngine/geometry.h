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

	Point2d& operator < (Point2d const& obj) {

	}

	bool operator == (Point2d const& obj) {
		return x == obj.x && y == obj.y;
	}

	int Displacement(Point2d p) { return (int)sqrt(abs((int)(x - p.x)) * abs((int)(x - p.x)) + abs((int)(y - p.y)) * abs((int)(y - p.y))); };
};

class Point3d {

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

int ComparePointsByCoordinate(uint8_t compare_type, std::vector<Point2d*>* v = nullptr, Point2d* p1 = nullptr, Point2d* p2 = nullptr, int begin = -1, int end = -1) {
	
	// Checking for invalid compare_type values
	if (0b11110000 & compare_type || 0b1000 & compare_type || 0b0000 & compare_type)
		return -1;
	if (0b10 & compare_type && 0b100 & compare_type)
		return -1;

	if (v) {
		return ComparePointVectorByCoordinate(compare_type, v, begin, end);
	}
	if (p1 && p2) {
		return ComparePointsByCoordinate(compare_type, p1, p2);
	}
	return -1;
}

int ComparePointVectorByCoordinate(uint8_t compare_type, std::vector<Point2d*>* v, int begin = -1, int end = -1) {

	uint16_t range_begin, range_end;
	uint8_t compare[4] = { 0b1, 0b10, 0b100, 0b1000 };

	uint32_t compare_value = (compare[0] & compare_type) ? UINT32_MAX : 0;
	int index = -1;
	uint32_t current;

	begin == -1 ? range_begin = 0 : range_begin = begin;
	end == -1 ? range_end = v->size() : range_end = end;
	for (uint8_t i = range_begin; i < range_end; i++) {
		Point2d p = *v->at(i);
		current = compare_type >> 1 * p.x + compare_type >> 2 * p.y;
		// Test teh selected coordinate value against the compare value
		if (((compare[0] & compare_type) && current > compare_value) || (!(compare[0] & compare_type) && current < compare_value)) {
			compare_value = current;
			index = i;
		}
	}

	return index;
}

int ComparePointsByCoordinate(uint8_t compare_type, Point2d* p1, Point2d* p2) {

	uint8_t compare[4] = { 0b1, 0b10, 0b100, 0b1000 };

	uint32_t compare_value[2];
	compare_value[0] = (compare[1] & compare_type) * p1->x + (compare[2] & compare_type) * p1->y;
	compare_value[1] = (compare[1] & compare_type) * p2->x + (compare[2] & compare_type) * p2->y;

	int index = -1;
	if (compare[0] & compare_type) {
		(compare_value[0] >= compare_value[1]) ? index = 0 : index = 1;
	}
	else {
		(compare_value[0] <= compare_value[1]) ? index = 0 : index = 1;
	}

	return index; 
}

void SwapPoint(Point2d& p1, Point2d& p2) {
	Point2d* temp;
	temp = &p1;
	p1 = p2;
	p2 = *temp;
}

// Compare two points by X coordinate value.
// Return value indicates whether a change occurred.
bool SortPointByX(Point2d& a, Point2d& b) {
	Point2d temp;
	if (a.x < b.x) {
		temp = a;
		a = b;
		b = temp;
		return true;
	}
	return false;
}

// Compare two points by Y coordinate value.
// Return value indicates whether a change occurred.
bool SortPointByY(Point2d& a, Point2d& b) {
	Point2d temp;
	if (a.y < b.y) {
		temp = a;
		a = b;
		b = temp;
		return true;
	}
	return false;
}

class Geometry {

public:
	struct GeometryData {
		// CURRENTLY STORING ALL VERTICES TWICE - COULD BE REFACTORED TO WORK ONLY ON A VECTOR INPUT
		void* handle; // corresponds to bottom left corner/edge
		int type;
		std::vector<Point2d> vertices;

		GeometryData& operator = (GeometryData const& obj) {
			handle = obj.handle;
			type = obj.type;
			vertices = obj.vertices;
			return *this;
		}

		GeometryData() { handle = nullptr; type = -1; };
		GeometryData(void* p_handle, int p_type, std::vector<Point2d> p_vertices) { handle = p_handle; type = p_type; vertices = p_vertices; };
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

	enum class compare {
		COMPARE_X_LO = 0b0000'0010,
		COMPARE_X_HI = 0b0000'0011,
		COMPARE_Y_LO = 0b0000'0100,
		COMPARE_Y_HI = 0b0000'0101,
		COMPARE_Z_LO = 0b0000'1000,
		COMPARE_Z_HI = 0b0000'1001,
	};
	
	// Collision function, must be implemented based on geometry type
	virtual bool Collision(Point2d p) = 0;
};

class Line : public Geometry {

public:
	Point2d a, b;
	GeometryData gd;

	Line() { Point2d a; Point2d b; GeometryData gd; };
	Line(Point2d p_a, Point2d p_b) { a.x = p_a.x; a.y = p_a.y; b.x = p_b.x; b.y = p_b.y; gd = GeometryData(); gd.vertices.push_back(a); gd.vertices.push_back(b); gd.type = G_LINE; };
	Line(Point2d p_a, Point2d p_b, void* p_handle, int p_type) { a.x = p_a.x; a.y = p_a.y; b.x = p_b.x; b.y = p_b.y; std::vector<Point2d> v; v.push_back(a); v.push_back(b); gd = GeometryData(p_handle, p_type, v); };

	Line& operator = (Line const& obj) {
		a = obj.a;
		b = obj.b;
		gd = obj.gd;
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
	Point2d LB, RT;

	Rect() { LT = Point2d(); RB = Point2d(); LB = Point2d(); RT = Point2d(); };
	Rect(Point2d a, Point2d b) { LT = Point2d(min(a.x, b.x), min(a.y, b.y)); RB = Point2d(max(a.x, b.x), max(a.y, b.y)); LB = Point2d(min(a.x, b.x), max(a.y, b.y)); RT = Point2d(max(a.x, b.x), min(a.y, b.y)); }; // does not assume that a is LT and b is RB
	Rect(RECT rect) { LT = Point2d(rect.left, rect.top); RB = Point2d(rect.right, rect.bottom); LB = Point2d(rect.left, rect.bottom); RT = Point2d(rect.right, rect.top); };
	Rect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) { LT = Point2d(left, top); RB = Point2d(right, bottom); LB = Point2d(left, bottom); RT = Point2d(right, top);
	};

	Rect& operator = (Rect const& obj) {
		LT = obj.LT;
		RB = obj.RB;
		LB = obj.LB;
		RT = obj.RT;
		return *this;
	}

	Rect& operator = (RECT const& obj) {
		LT = Point2d(obj.left, obj.top);
		RB = Point2d(obj.right, obj.bottom);
		LB = Point2d(obj.left, obj.bottom);
		RT = Point2d(obj.right, obj.top);
		return *this;
	}

	int GetWidth() { return this->RB.x - this->LT.x; };
	int GetHeight() { return this->RB.y - this->LT.y; };
	bool Collision(Point2d p) override { return (p.x >= LT.x && p.x <= RB.x && p.y >= LT.y && p.y <= RB.y); }
};

class Quad : public Geometry {
	Point2d a, b, c, d;

	bool Collision(Point2d p) override { return false; }
};

class Circle : public Geometry {
	Point2d c;
	uint16_t r = 0;

	bool Collision(Point2d p) override { return c.Displacement(p) <= r; }
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