#include "quadtree.h"
#include <iostream>

Quadtree::Quadtree(const Rect& r) {
	root = new Quadrant(r.lb.x, r.lb.y, r.rt.x, r.rt.y);
}

Quadtree::~Quadtree() {

	for (const auto l : gridLines) {
		delete l;
	}
	delete root;
}

Quadtree::Quadrant::~Quadrant() {

	for (Quadrant* c : children) {
		delete c;
	}
	delete p;
}

Point2d* Quadtree::getPoint(Quadrant* q) {
	return q->getPoint();
}

void Quadtree::Quadrant::assignPoint(const Point2d& p_p, std::vector<Line*>& lines) {
	
	// Assign point to empty quadrant
	if (!p) {
		p = new Point2d(p_p);
		return;
	}
	// If p already exists, divide this quadrant into 4 subsections
	uint32_t halfWidthIndex;
	uint32_t halfHeightIndex;

	halfWidthIndex = getWidth() / 2;
	halfHeightIndex = getHeight() / 2;

	// Adjust for odd number dimensions
	if (getWidth() % 2 == 0)
		halfWidthIndex--;
	if (getHeight() % 2 == 0)
		halfHeightIndex--;

	// Store the crossed gridlines each time we identify a new quadrant to segment (since the lines are 1 pixel apart for each quad, we include
	// all four lines required to draw the cross pattern)
	// Vertical left of center
	lines.push_back(new Line(Point2d(left + halfWidthIndex, top), Point2d(left + halfWidthIndex, bottom)));
	// Right of center
	lines.push_back(new Line(Point2d(left + halfWidthIndex + 1, top), Point2d(left + halfWidthIndex + 1, bottom)));
	// Horizontal top of center
	lines.push_back(new Line(Point2d(left, bottom - halfHeightIndex - 1), Point2d(right, bottom - halfHeightIndex - 1)));
	// Bottom of center
	lines.push_back(new Line(Point2d(left, bottom - halfHeightIndex), Point2d(right, bottom - halfHeightIndex)));

	// Seqment current quad into 4 children
	Quadrant* bottomLeft = new Quadrant(left, bottom, left + halfWidthIndex, bottom - halfHeightIndex);
	children.push_back(bottomLeft);
	Quadrant* topLeft = new Quadrant(left, bottom - halfHeightIndex - 1, left + halfWidthIndex, top);
	children.push_back(topLeft);
	Quadrant* topRight = new Quadrant(left + halfWidthIndex + 1, bottom - halfHeightIndex - 1, right, top);
	children.push_back(topRight);
	Quadrant* bottomRight = new Quadrant(left + halfWidthIndex + 1, bottom, right, bottom - halfHeightIndex);
	children.push_back(bottomRight);

	// Look for the appropriate quadrant to re-assign p
	// Recursively assign p_p until it lands in a quadrant that is empty
	int index_p_p = -1;
	for (int i = 0; i < 4; i++) {
		if (p && children.at(i)->collidesWith(*p)) {
			children.at(i)->p = p;
			this->p = nullptr;
		}
		if (children.at(i)->collidesWith(p_p))
			index_p_p = i;
	}
	if (index_p_p != -1) {
		children.at(index_p_p)->assignPoint(p_p, lines);
		return;
	}

	Debug::DebugMessage msg = Debug::DebugMessage(CallingClasses::QUADTREE_CLASS, DebugTypes::QUADRANT_NOT_FOUND);
	msg.Print();
}

void Quadtree::Quadrant::unassignPoint(const Point2d& p_p, std::vector<Line*>& lines) {

	// logic for removal is a little more complicated than addition
	// need to account for other points in our parent quad that may remain, thus preventing removal of grid lines
}

Point2d* Quadtree::Quadrant::getPoint() {

	return p;
}

std::vector<Quadtree::Quadrant*> Quadtree::Quadrant::getChildren() {

	return children;
}

Quadtree::Quadrant* Quadtree::Quadrant::findChildQuadrant(const Point2d& p_p) {
	bool right = false, top = false;
	if (p_p.x >= children.at(3)->left) {
		right = true;
	}
	if (p_p.y <= children.at(2)->bottom) {
		top = true;
	}
	// Left quadrants -> right and top
	// Right quadrants -> right and bottom
	// enforces natural binary conversion from a two bit binary integer back to indices for children
	int mask = 0b00;
	mask |= (int)right << 1;
	right ? mask |= (int)!top << 0 : mask |= (int)top << 0;
	return children.at(mask);
}

void Quadtree::updateGridLines(std::vector<Geometry*> queue) {

	gridLines.clear();
	for (Geometry* g : queue) {
		for (Point2d p : g->vertices) {
			assignPoint(p);
		}
	}
}

void Quadtree::addGeometry(Geometry* g) {

	int counter = 1;
	for (Point2d p : g->vertices) {
		assignPoint(p);
		std::cout << "Added point " << counter++ << std::endl;
	}
}

void Quadtree::removeGeometry(Geometry* g) {

	for (Point2d p : g->vertices) {
		unassignPoint(p);
	}
}

void Quadtree::assignPoint(const Point2d& p) {
	
	Quadrant* ptr = root;
	// While the current quadrant has children, step down until we find a viable, non-divided (and possibly empty) quadrant to place point
	while (ptr->isParent()) {
		Quadrant* next = ptr->findChildQuadrant(p);
		ptr = next;
	}
 	ptr->assignPoint(p, gridLines);
}

void Quadtree::unassignPoint(const Point2d& p) {

	Quadrant* ptr = root;
	while (ptr->isParent()) {
		Quadrant* next = ptr->findChildQuadrant(p);
		ptr = next;
	}
	ptr->unassignPoint(p, gridLines);
}

std::vector<Line*>* Quadtree::getQuadtreeGrid() {
	
	return &gridLines;
}

std::string Quadtree::toString() {
	Quadrant* q = root;
	int depth = 0;
	std::string output = q->toString(depth);

	return output;
}

std::string Quadtree::Quadrant::toString(int depth) {

	std::ostringstream output;
	std::string indent = "";
	for (int i = 0; i < depth; i++) {
		indent += "\t";
		//output << "\t";
	}
	output << indent;
	output << "Left: " << this->getLeft();
	output << " || Top: " << this->getTop();
	output << " || Right: " << this->getRight();
	output << " || Bottom: " << this->getBottom() << "\n";
	if (p) {
		output << indent << "\tp: X " << p->x << " : Y " << p->y << "\n";
	}

	if (isParent()) {
		for (int i = 0; i < children.size(); i++) {
			output << children.at(i)->toString(depth + 1);
		}
	}

	return output.str();
}