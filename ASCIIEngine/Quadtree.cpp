#include "quadtree.h"
#include <iostream>

Quadtree::Quadtree(const Rect& r) {
	root = new Quadrant(r.lb.x, r.lb.y, r.rt.x, r.rt.y, nullptr);
}

Quadtree::~Quadtree() {

	for (const auto l : gridLines) {
		delete l;
	}
	delete root;
}

Quadtree::Quadrant::Quadrant(uint32_t left, uint32_t bottom, uint32_t right, uint32_t top, Quadrant* parent) {
	this->left = left;
	this->right = right;
	this->top = top;
	this->bottom = bottom;
	this->parent = parent;
	reset();
};

Quadtree::Quadrant::~Quadrant() {

	for (Quadrant* c : children) {
		delete c;
	}
	delete p;
}

void Quadtree::Quadrant::assignPoint(const Point2d& p_p, std::vector<Line*>& lines) {
	
	// Assign point to empty quadrant
	if (!p) {
		p = new Point2d(p_p);
		return;
	}
	// If p already exists, divide this quadrant into 4 subsections
	segmentQuadrant(lines);

	// Look for the appropriate quadrant to re-assign p
	// Recursively assign p_p until it lands in a quadrant that is empty
	int index_p_p = -1;
	for (int i = 0; i < 4; i++) {
		if (p && children.at(i)->collidesWith(*p)) {
			reassignPoint(this, children.at(i));
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

//bool Quadtree::Quadrant::unassignPoint(const Point2d& p_p) {
//	p = nullptr;
//
//	int count = 0;
//	for (Quadrant* q : parent->children) {
//		if (q->sumValidPointsRemaining()) {
//			// If more than one valid point remains, we can't remove the grid lines
//			return false;
//		}
//	}
//	return true;
//}

uint16_t Quadtree::Quadrant::sumValidPointsRemaining() {
	uint16_t count = 0;
	if (p) {
		count++;
	}
	for (Quadrant* q : children) {
		count += q->sumValidPointsRemaining();
		if (count > 1) {
			// return early if enough points are found
			return count;
		}
	}
	return count;
}

bool Quadtree::Quadrant::linesEligibleForRemoval() {
	for (Quadrant* q : children) {
		if (q->sumValidPointsRemaining() > 1) {
			return false;
		}
	}
	return true;
}

void Quadtree::Quadrant::segmentQuadrant(std::vector<Line*>& lines) {
	uint32_t halfWidthIndex = getWidth() / 2;
	uint32_t halfHeightIndex = getHeight() / 2;

	// Adjust for odd number dimensions
	if (getWidth() % 2 == 0)
		halfWidthIndex--;
	if (getHeight() % 2 == 0)
		halfHeightIndex--;

	// Store the crossed gridlines each time we identify a new quadrant to segment (since the lines are 1 pixel apart for each quad, we include
	// all four lines required to draw the cross pattern)
	// Vertical left of center
	Line* verticalLeftOfCenter = new Line(Point2d(left + halfWidthIndex, top), Point2d(left + halfWidthIndex, bottom));
	lines.push_back(verticalLeftOfCenter);
	quadrantLines.push_back(verticalLeftOfCenter);
	// Right of center
	Line* verticalRightOfCenter = new Line(Point2d(left + halfWidthIndex + 1, top), Point2d(left + halfWidthIndex + 1, bottom));
	lines.push_back(verticalRightOfCenter);
	quadrantLines.push_back(verticalRightOfCenter);
	// Horizontal top of center
	Line* horizontalTopOfCenter = new Line(Point2d(left, bottom - halfHeightIndex - 1), Point2d(right, bottom - halfHeightIndex - 1));
	lines.push_back(horizontalTopOfCenter);
	quadrantLines.push_back(horizontalTopOfCenter);
	// Bottom of center
	Line* horizontalBottomOfCenter = new Line(Point2d(left, bottom - halfHeightIndex), Point2d(right, bottom - halfHeightIndex));
	lines.push_back(horizontalBottomOfCenter);
	quadrantLines.push_back(horizontalBottomOfCenter);

	// Seqment current quad into 4 children
	Quadrant* bottomLeft = new Quadrant(left, bottom, left + halfWidthIndex, bottom - halfHeightIndex, this);
	children.push_back(bottomLeft);
	Quadrant* topLeft = new Quadrant(left, bottom - halfHeightIndex - 1, left + halfWidthIndex, top, this);
	children.push_back(topLeft);
	Quadrant* topRight = new Quadrant(left + halfWidthIndex + 1, bottom - halfHeightIndex - 1, right, top, this);
	children.push_back(topRight);
	Quadrant* bottomRight = new Quadrant(left + halfWidthIndex + 1, bottom, right, bottom - halfHeightIndex, this);
	children.push_back(bottomRight);
}

void Quadtree::Quadrant::unsegmentQuadrant(Quadrant* parent) {
	for (Line* l : this->parent->quadrantLines) {
		delete l;
	}
	for (Quadrant* q : this->parent->children) {
		delete q;
	}
	parent->reset();
}

void Quadtree::Quadrant::reassignPoint(Quadrant* src, Quadrant* dst) {
	dst->p = p;
	src->p = nullptr;
}

Quadtree::Quadrant* Quadtree::Quadrant::findChildQuadrant(const Point2d& p_p) {
	bool right = false, top = false;
	if (p_p.x >= children.at(3)->left) {
		right = true;
	}
	if (p_p.y <= children.at(2)->bottom) {
		top = true;
	}
	// enforces natural binary conversion from a two bit binary integer back to indices for children
	int mask = 0b00;
	mask |= (int)right << 1;
	right ? mask |= (int)!top << 0 : mask |= (int)top << 0;
	return children.at(mask);
}

void Quadtree::addGeometry(Geometry* g) {
	for (Point2d p : g->vertices) {
		assignPoint(p);
	}
}

void Quadtree::removeGeometry(Geometry* g) {
	Quadrant* parentOfRemoved = nullptr;
	for (Point2d p : g->vertices) {
		parentOfRemoved = unassignPoint(p);
	}

	// Now that all vertices are removed, can we remove this quadrant, its siblings, and the gridlines?
	if (parentOfRemoved) {
		while (parentOfRemoved->linesEligibleForRemoval()) {
			for (Line* l : parentOfRemoved/*->parent*/->quadrantLines) {
				delete l;
				gridLines.pop_back();
			}
			for (Quadrant* q : parentOfRemoved/*->parent*/->children) {
				delete q;
			}
			parentOfRemoved->reset();
			if (parentOfRemoved->parent == nullptr) {
				break;
			}

			parentOfRemoved = parentOfRemoved->parent;
		}
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

Quadtree::Quadrant* Quadtree::unassignPoint(const Point2d& p) {
	// Step down to the quadrant containing the point in question
	Quadrant* ptr = root;
	while (ptr->isParent()) {
		Quadrant* next = ptr->findChildQuadrant(p);
		ptr = next;
	}
	// Null the point being removed
	ptr->p = nullptr;
	if (ptr->parent) {
		return ptr->parent;
	}
	return nullptr;
}

void Quadtree::Quadrant::reset() {
	children = std::vector<Quadrant*>(0);
	quadrantLines = std::vector<Line*>(0);
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
	output << "Left: " << this->left;
	output << " || Top: " << this->top;
	output << " || Right: " << this->right;
	output << " || Bottom: " << this->bottom << "\n";
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