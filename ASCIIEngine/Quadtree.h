#ifndef ASCIIENGINE_QUADTREE_H_
#define ASCIIENGINE_QUADTREE_H_

#include <cstdint>
#include "geometry.h"
#include <sstream>

class Quadtree {

public:
	Quadtree(Rect r);
	~Quadtree();

private:
	std::vector<Line*> gridLines;
	struct Quadrant {
		// X and Y positions referenced from bottom left to top right (matches draw_area_ indexing)
		uint32_t left;
		uint32_t right;
		uint32_t top;
		uint32_t bottom;
		Point2d* p = nullptr;
		//Quadrant* parent;
		std::vector<Quadrant*> children; // Quadrants indexed bottom left, clockwise around
		uint32_t getLeft() { return left; };
		uint32_t getRight() { return right; };
		uint32_t getTop() { return top; };
		uint32_t getBottom() { return bottom; };

	public:
		Quadrant() { left = 0; right = 0; top = 0; bottom = 0; };
		Quadrant(uint32_t left, uint32_t bottom, uint32_t right, uint32_t top) { this->left = left; this->right = right; this->top = top; this->bottom = bottom; };
		~Quadrant();
		
		void assignPoint(Point2d* p_p, std::vector<Line*>* lines);
		void unassignPoint(Point2d* p_p, std::vector<Line*>* lines);
		uint32_t getWidth() { return right - left; };
		uint32_t getHeight() { return bottom - top; };

		Point2d* getPoint();
		std::vector<Quadrant*> getChildren();
		bool collidesWith(Point2d* p) { return (p->x >= left && p->x <= right && p->y >= top && p->y <= bottom); }; // top and bottom refer to their respective locales on screen but the row count is reversed due to how the draw area is index
		bool isParent() { return children.size() > 0; };
		Quadrant* findChildQuadrant(Point2d* p_p);
		void getGridLines(std::vector<Line*>& quadtree_lines);
		std::string toString(int depth);
	};
	Quadrant* root;

protected:
	Point2d* getPoint(Quadrant* q);
	std::vector<Line*> getGridLines();
	void assignPoint(Point2d* p);
	void unassignPoint(Point2d* p);
	
public:
	void updateGridLines(std::vector<Geometry*> queue);
	void addGeometry(Geometry* g);
	void removeGeometry(Geometry* g);

	std::vector<Line*>* getQuadtreeGrid();
	void getQuadtreeGrid(Quadrant* ptr, std::vector<Line*> lines);
	std::string toString();
};


#endif // !ASCIIENGINE_QUADTREE_H_