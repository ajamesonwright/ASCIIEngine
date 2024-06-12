#ifndef ASCIIENGINE_QUADTREE_H_
#define ASCIIENGINE_QUADTREE_H_

#include <cstdint>
#include "Geometry.h"
#include <sstream>

class Quadtree {

public:
	Quadtree(Rect r);
	~Quadtree();

private:
	std::vector<Line*> gridLines;
	class Quadrant {
		// X and Y positions referenced from bottom left to top right (matches draw_area_ indexing)
		uint32_t x0, x1, y0, y1;
		Point2d* p = nullptr;
		//Quadrant* parent;
		std::vector<Quadrant*> children; // Quadrants indexed bottom left, clockwise around
		uint32_t getLeft() { return x0; };
		uint32_t getRight() { return x1; };
		uint32_t getTop() { return y0; };
		uint32_t getBottom() { return y1; };

	public:
		Quadrant() { x0 = 0; x1 = 0; y0 = 0; y1 = 0; };
		Quadrant(uint32_t p_x0, uint32_t p_y0, uint32_t p_x1, uint32_t p_y1) { x0 = p_x0; x1 = p_x1; y0 = p_y0; y1 = p_y1; };
		~Quadrant();
		
		void assignPoint(Point2d* p_p, std::vector<Line*>* lines);
		void unassignPoint(Point2d* p_p, std::vector<Line*>* lines);
		uint32_t getWidth() { return x1 - x0; };
		uint32_t getHeight() { return y0 - y1; };

		Point2d* getPoint();
		std::vector<Quadrant*> getChildren();
		bool collidesWith(Point2d* p) { return (p->x >= x0 && p->x <= x1 && p->y <= y0 && p->y >= y1); };
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