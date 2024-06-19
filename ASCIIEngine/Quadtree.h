#ifndef ASCIIENGINE_QUADTREE_H_
#define ASCIIENGINE_QUADTREE_H_

#include <cstdint>
#include "geometry.h"
#include <sstream>

class Quadtree {

private:
	struct Quadrant {
		uint32_t left;
		uint32_t right;
		uint32_t top;
		uint32_t bottom;
		Point2d* p = nullptr;
		Quadrant* parent = nullptr;
		std::vector<Line*> quadrantLines;
		std::vector<Quadrant*> children; // Quadrants indexed bottom left, clockwise around

		Quadrant(uint32_t left, uint32_t bottom, uint32_t right, uint32_t top, Quadrant* parent);
		~Quadrant();

		void assignPoint(const Point2d& p_p, std::vector<Line*>& lines);
		//bool unassignPoint(const Point2d& p_p);
		bool linesEligibleForRemoval();
		void unsegmentQuadrant(Quadrant* parent);

		Quadrant* findChildQuadrant(const Point2d& p_p);

		bool collidesWith(const Point2d& p) { return (p.x >= left && p.x <= right && p.y >= top && p.y <= bottom); }; // top and bottom refer to their respective locales on screen but the row count is reversed due to how the draw area is index
		bool isParent() { return children.size() > 0; };
		std::string toString(int depth);
		void reset();

		bool operator==(Quadrant& const obj) const {
			return left == obj.left && right == obj.right && top == obj.top && bottom == obj.bottom;
		}

	private:
		uint16_t sumValidPointsRemaining();
		void segmentQuadrant(std::vector<Line*>& lines);
		void reassignPoint(Quadrant* src, Quadrant* dst);
		uint32_t getHeight() { return bottom - top; };
		uint32_t getWidth() { return right - left; };
	};

public:
	Quadtree(const Rect& r);
	~Quadtree();

	void addGeometry(Geometry* g);
	void removeGeometry(Geometry* g);
	std::vector<Line*>* getQuadtreeGrid();
	std::string toString();

protected:
	void assignPoint(const Point2d& p);
	Quadrant* unassignPoint(const Point2d& p);

	Quadrant* root;
	std::vector<Line*> gridLines;
};


#endif // !ASCIIENGINE_QUADTREE_H_