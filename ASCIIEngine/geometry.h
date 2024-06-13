#ifndef ASCIIENGINE_GEOMETRY_H_
#define ASCIIENGINE_GEOMETRY_H_

#include <Windows.h>
#include <stdint.h>
#include <vector>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include "debug.h"

class Point2d {

public:
	uint32_t x, y;

	Point2d() { x = 0; y = 0; };
	Point2d(POINT p) { x = p.x; y = p.y; };
	Point2d(const Point2d& source) { x = source.x; y = source.y; };
	Point2d(uint32_t p_x, uint32_t p_y) { x = p_x; y = p_y; };

	//Point2d& operator = (Point2d const& obj) {
	//	x = obj.x;
	//	y = obj.y;
	//	return *this;
	//}

	bool operator == (Point2d const& obj) {
		return x == obj.x && y == obj.y;
	}

	int displacementFrom(Point2d p) { return (int)sqrt(abs((int)(x - p.x)) * abs((int)(x - p.x)) + abs((int)(y - p.y)) * abs((int)(y - p.y))); };
};

class Point3d : public Point2d {

public:
	uint32_t z;

	Point3d() { z = 0; };
	Point3d(const Point3d& source) { x = source.x; y = source.y; z = source.z; };
	Point3d(Point2d p, uint32_t p_z) { x = p.x; y = p.y; z = p_z; };

	bool operator == (Point3d const& obj) {
		return x == obj.x && y == obj.y && z == obj.z;
	}
};

class Geometry {

public:
	int type;
	std::vector<Point2d*> vertices;

	Geometry() { type = -1; };
	Geometry(int p_type) { type = p_type; };
	Geometry(const Geometry& source) {

		type = source.type;
		vertices.clear();
		for (int i = 0; i < source.vertices.size(); i++) {
			vertices.push_back(source.vertices[i]);
		}
	};

	enum GeometryType {
		G_LINE,
		G_TRI,
		G_RECT,
		G_QUAD,
		G_CIRCLE,

		G_NUM_TYPES,
	};
	
	virtual bool collidesWith(Point2d p) = 0;

protected:
	int comparePointsByCoordinate(const uint8_t compare_type, const std::vector<Point2d*>* v = nullptr, const Point2d* p1 = nullptr, const Point2d* p2 = nullptr, const int begin = -1, const int end = -1);
	std::vector<float> calculateSlopes(const std::vector<Point2d*> v_g);
	void sortBySlope(std::vector<Point2d*> &vertices, const std::vector<float> slopes);
private:
	int comparePointVectorByCoordinate(const uint8_t compare_type, const std::vector<Point2d*>* v, const int begin = -1, const int end = -1);
	int comparePointPairByCoordinate(const uint8_t compare_type, const Point2d* p1, const Point2d* p2);
	bool compareBySlope(float f1, float f2);
};

class Line : public Geometry {

public:
	Line() : Geometry(G_LINE) {};
	Line(const Line& source) {

		type = source.type;
		vertices.clear();
		vertices.push_back(source.vertices[0]);
		vertices.push_back(source.vertices[1]);
	};
	Line(const Geometry& source) {

		if (source.type != G_LINE || source.vertices.size() != 2)
			return;
		type = source.type;
		vertices.clear();
		vertices.push_back(source.vertices[0]);
		vertices.push_back(source.vertices[1]);
	};
	Line(Point2d a, Point2d b) : Geometry(G_LINE) {

		if (a == b)
			return;

		int index_y = comparePointsByCoordinate(0b101, nullptr, &a, &b);
		int index_x = comparePointsByCoordinate(0b10, nullptr, &a, &b);
		
		vertices.clear();
		if (index_y != -1) {
			if (index_y == 0) {
				vertices.push_back(new Point2d(a));
				vertices.push_back(new Point2d(b));
			} else {
				vertices.push_back(new Point2d(b));
				vertices.push_back(new Point2d(a));
			}
			return;
		}
		
		// index_x cannot be -1 otherwise the return statement at the top has already executed
		if (index_x == 0) {
			vertices.push_back(new Point2d(a));
			vertices.push_back(new Point2d(b));
			return;
		}
		vertices.push_back(new Point2d(b));
		vertices.push_back(new Point2d(a));
	};

	bool collidesWith(Point2d p) { return false; }
};

class Tri : public Geometry {

public:
	Tri() : Geometry(G_TRI) {};
	Tri(const Tri& source) : Geometry(G_TRI) {

		type = source.type;
		vertices.clear();
		for (int i = 0; i < source.vertices.size(); i++) {
			vertices.push_back(source.vertices[i]);
		}
	};
	Tri(const Geometry& source) {

		if (source.type != G_TRI || source.vertices.size() != 3)
			return;

		type = source.type;
		vertices.clear();
		for (int i = 0; i < source.vertices.size(); i++) {
			vertices.push_back(source.vertices[i]);
		}
	};
	Tri(Point2d a, Point2d b, Point2d c) : Geometry(G_TRI) {

		vertices.clear();
		vertices.push_back(&a); vertices.push_back(&b); vertices.push_back(&c);
		int index = comparePointsByCoordinate(0b101, &vertices);
		if (index != 0)
			std::swap(vertices.at(0), vertices.at(index));
		// currently would not account for points with the same x-coord (TODO: implement binary int return type and masking for decode)
		index = comparePointsByCoordinate(0b10, &vertices, nullptr, nullptr, 1);
		if (index != 1)
			std::swap(*vertices.at(1), *vertices.at(index));
	};

	bool collidesWith(Point2d p) { return false; }
};

// Four-sided shape that assumes orthogonal sides 
class Rect : public Geometry {

public:
	Point2d lt, rb;
	Point2d lb, rt;

	Rect() : Geometry(G_RECT) { lt = Point2d(); rb = Point2d(); lb = Point2d(); rt = Point2d(); vertices.clear(); };
	Rect(const Rect& source) {

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
	Rect(const Geometry& source) {

		if (source.type != G_RECT || source.vertices.size() != 4)
			return;

		type = source.type;
		vertices.clear();
		for (int i = 0; i < source.vertices.size(); i++) {
			vertices.push_back(source.vertices[i]);
		}
		lb = *vertices.at(0); // Ordering of pairs is enforced from creation by points, therefore existing pattern can be used to determine order instead of testing
		lt = *vertices.at(1);
		rt = *vertices.at(2);
		rb = *vertices.at(3);
	};
	Rect(Point2d a, Point2d b) : Geometry(G_RECT) {

		lb = Point2d(min(a.x, b.x), max(a.y, b.y));
		lt = Point2d(min(a.x, b.x), min(a.y, b.y));
		rt = Point2d(max(a.x, b.x), min(a.y, b.y));
		rb = Point2d(max(a.x, b.x), max(a.y, b.y));
		vertices.clear();
		vertices.push_back(&lb);
		vertices.push_back(&lt);
		vertices.push_back(&rt);
		vertices.push_back(&rb);
	};
	Rect(RECT rect) : Geometry(G_RECT) {

		lb = Point2d(rect.left, rect.bottom);
		lt = Point2d(rect.left, rect.top);
		rt = Point2d(rect.right, rect.top); 
		rb = Point2d(rect.right, rect.bottom);
		vertices.clear();
		vertices.push_back(&lb);
		vertices.push_back(&lt);
		vertices.push_back(&rt);
		vertices.push_back(&rb);		
	};
	Rect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) : Geometry(G_RECT) {
				
		lb = Point2d(left, bottom); 
		lt = Point2d(left, top); 
		rt = Point2d(right, top);
		rb = Point2d(right, bottom);
		vertices.clear();
		vertices.push_back(&lb);
		vertices.push_back(&lt);
		vertices.push_back(&rt);
		vertices.push_back(&rb);
	};

	int getWidth() { return this->rb.x - this->lt.x; };
	int getHeight() { return this->rb.y - this->lt.y; };
	bool collidesWith(Point2d p) { return (p.x >= lt.x && p.x <= rb.x && p.y >= lt.y && p.y <= rb.y); }
};

class Quad : public Geometry {
	
	Quad() {};
	Quad(const Quad& source) {

		if (source.type != G_QUAD || source.vertices.size() != 4)
			return;

		type = source.type;
		vertices.clear();
		for (int i = 0; i < source.vertices.size(); i++) {
			vertices.push_back(source.vertices[i]);
		}
	};
	Quad(const Geometry& source) {

		if (source.type != G_QUAD || source.vertices.size() != 4)
			return;

		type = source.type;
		vertices.clear();
		for (int i = 0; i < source.vertices.size(); i++) {
			vertices.push_back(source.vertices[i]);
		}
	};
	Quad(Point2d a, Point2d b, Point2d c, Point2d d) : Geometry(G_QUAD) {

		//// USE CONVEX HULL APPROACH TO SORT POINTS
		//// FIND LOWEST Y COORD
		//// CALCULATE SLOPE OF LINE FROM THAT POINT TO EACH OTHER POINT AND SORT 
		//// should be sorted s.th. least negative -> most negative -> 1 -> most positive -> least positive
		//vertices.push_back(&a);
		//vertices.push_back(&b);
		//vertices.push_back(&c);
		//vertices.push_back(&d);

		//int index_y = ComparePointsByCoordinate(0b101, &vertices);
		//if (index_y != 0)
		//	std::swap(*vertices.at(0), *vertices.at(index_y));
		//
		//std::vector<float> slope_vector = CalculateSlopes(vertices);
		//SortBySlope(vertices, slope_vector);
		///*for (int i = 0; i < slope_vector.size() - 1; i++) {
		//	if (slope_vector[i] < 0 && slope_vector[i+1] < 0)
		//		slope_vector[i] < slope_vector[i+1] ? 
		//}*/
	}

	bool collidesWith(Point2d p) { return false; }
};

class Circle : public Geometry {

public:
	Point2d center;
	uint16_t r = 0;

	Circle() { center = Point2d(); r = 0; };
	Circle(Point2d p_c, uint16_t p_r) { center = p_c; r = p_r; type = G_CIRCLE; vertices.push_back(&p_c); };
	Circle(const Circle& source) : Geometry(G_CIRCLE) {
		center = source.center;
		r = source.r;
		vertices.push_back(source.vertices[0]);
	}
	Circle(const Geometry& source) {
		if (source.type != G_CIRCLE && source.vertices.size() != 1)
			return;

		type = source.type;
		center = *source.vertices[0];
		vertices.push_back(&center);
		r = 30;
	}
	bool collidesWith(Point2d p) { return center.displacementFrom(p) <= r; }
};

class Ray2d : public Point2d {

public:
	float direction; // 0-360 degrees, 0 degrees aligned with positive x-axis, positive rotation moving towards positive y-axis
	
	Ray2d() { x = 0; y = 0; direction = 0.0f; };
	Ray2d(const Ray2d& source) { x = source.x; y = source.y; direction = source.direction; };
	Ray2d(uint32_t p_x, uint32_t p_y, float p_direction) { x = p_x; y = p_y; direction = p_direction; };
};

class Camera : public Ray2d {

	// Visual representation in top down panel, using an arrow to depict position and direction
public:
	uint8_t size; 
	float px, py, vx, vy, ax, ay;
	uint16_t turn_speed = 150, move_speed = 1000;
	Point2d left, right, tip, base;
	uint8_t fov = 60;
	uint8_t height = 1800;

	Camera() { x = 0; y = 0; direction = 0.0f; size = 10; px = 0.0f; py = 0.0f; vx = 0.0f; vy = 0.0f; ax = 0.0f; ay = 0.0f; };
	Camera(const Camera& source);
	Camera(uint32_t p_x, uint32_t p_y, float p_direction);

	void update();

	void setSize(uint8_t p_size) { size = p_size; };
	void clampDirection();
	void clampPosition(Point2d p, Geometry* g);
	void clampPosition(Rect panel);
	void clampVelocity();
	void clampAcceleration();
};

struct Ray3d : public Point3d {

public:
	uint16_t yaw, pitch;

	Ray3d() { x = 0; y = 0; z = 0; yaw = 0; pitch = 0; };
	Ray3d(const Ray3d& source) { x = source.x; y = source.y; z = source.z; yaw = source.yaw; pitch = source.pitch; };
	Ray3d(uint32_t p_x, uint32_t p_y, uint32_t p_z, uint16_t p_yaw, uint16_t p_pitch) { x = p_x; y = p_y; z = p_z; yaw = p_yaw; pitch = p_pitch; };
};

#endif