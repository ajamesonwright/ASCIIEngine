#include "Quadtree.h"
#include <iostream>

Quadtree::Quadtree(Rect r) {
	root = new Quadrant(r.lb.x, r.lb.y, r.rt.x, r.rt.y);
}

Quadtree::~Quadtree() {

	delete root;
}

Quadtree::Quadrant::~Quadrant() {

	for (Quadrant* c : children) {
		delete c;
	}
}

Point2d* Quadtree::GetPoint(Quadrant* q) {
	return q->getPoint();
}

void Quadtree::Quadrant::AssignPoint(Point2d* p_p, std::vector<Line*>* lines) {
	
	// Assign point to empty quadrant
	if (!p) {
		p = p_p;
		return;
	}
	// If p already exists, divide this quadrant into 4 subsections
	uint32_t half_width_index;
	uint32_t half_height_index;

	half_width_index = GetWidth() / 2;
	half_height_index = GetHeight() / 2;

	if (GetWidth() % 2 == 0)
		half_width_index--;
	if (GetHeight() % 2 == 0)
		half_height_index--;

	// Store the crossed gridlines each time we identify a new quadrant to segment (since the lines are 1 pixel apart for each quad, we include
	// all four lines required to draw the cross pattern)
	// Vertical
	Line* l0 = new Line(Point2d(x0 + half_width_index, y0), Point2d(x0 + half_width_index, y1));
	lines->push_back(l0);
	Line* l1 = new Line(Point2d(x0 + half_width_index + 1, y0), Point2d(x0 + half_width_index + 1, y1));
	lines->push_back(l1);
	// Horizontal
	Line* l2 = new Line(Point2d(x0, y0 - half_height_index), Point2d(x1, y0 - half_height_index));
	lines->push_back(l2);
	Line* l3 = new Line(Point2d(x0, y0 - half_height_index + 1), Point2d(x1, y0 - half_height_index + 1));
	lines->push_back(l3);


	// Seqment current quad into 4 children
	Quadrant* q0 = new Quadrant(x0, y0, x0 + half_width_index, y0 - half_height_index);
	children.push_back(q0);
	Quadrant* q1 = new Quadrant(x0, y0 - half_height_index - 1, x0 + half_width_index, y1);
	children.push_back(q1);
	Quadrant* q2 = new Quadrant(x0 + half_width_index + 1, y0 - half_height_index - 1, x1, y1);
	children.push_back(q2);
	Quadrant* q3 = new Quadrant(x0 + half_width_index + 1, y0, x1, y0 - half_height_index);
	children.push_back(q3);

	// Look for the appropriate quadrant to re-assign p
	// Recursively assign p_p until it lands in a quadrant that is empty
	int index_p_p = -1;
	for (int i = 0; i < 4; i++) {
		if (p && children.at(i)->Collision(p)) {
			children.at(i)->p = p;
			this->p = nullptr;
		}
		if (children.at(i)->Collision(p_p))
			index_p_p = i;
	}
	if (index_p_p != -1) {
		children.at(index_p_p)->AssignPoint(p_p, lines);
		return;
	}

	Debug::DebugMessage msg = Debug::DebugMessage(CallingClasses::QUADTREE_CLASS, DebugTypes::QUADRANT_NOT_FOUND);
	msg.Print();
}

void Quadtree::Quadrant::UnassignPoint(Point2d* p_p, std::vector<Line*>* lines) {

	// logic for removal is a little more complicated than addition
	// need to account for other points in our parent quad that may remain, thus preventing removal of grid lines
}

Point2d* Quadtree::Quadrant::getPoint() {

	return p;
}

std::vector<Quadtree::Quadrant*> Quadtree::Quadrant::getChildren() {

	return children;
}

Quadtree::Quadrant* Quadtree::Quadrant::FindChildQuadrant(Point2d* p_p) {
	bool right = false, top = false;
	if (p_p->x >= children.at(3)->x0) {
		right = true;
	}
	if (p_p->y <= children.at(2)->y0) {
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

//void Quadtree::Quadrant::GetGridLines(std::vector<Line*>& quadtree_lines) {
//	Line* left = new Line(Point2d(x0, y0), Point2d(x0, y1));
//	quadtree_lines.push_back(left);
//	//gridLines.push_back(left);
//	//delete left;
//	Line* top = new Line(Point2d(x0, y1), Point2d(x1, y1));
//	quadtree_lines.push_back(top);
//	//delete top;
//	Line* right = new Line(Point2d(x1, y1), Point2d(x1, y0));
//	quadtree_lines.push_back(right);
//	//delete right;
//	Line* bottom = new Line(Point2d(x1, y0), Point2d(x0, y0));
//	quadtree_lines.push_back(bottom);
//	//delete bottom;
//}

void Quadtree::UpdateGridLines(std::vector<Geometry*> queue) {

	gridLines.clear();
	for (Geometry* g : queue) {
		for (Point2d* p : g->vertices) {
			AssignPoint(p);
		}
	}
}

void Quadtree::AddGeometry(Geometry* g) {

	int counter = 1;
	for (Point2d* p : g->vertices) {
		AssignPoint(p);
		std::cout << "Added point " << counter++ << std::endl;
	}
}

void Quadtree::RemoveGeometry(Geometry* g) {

	for (Point2d* p : g->vertices) {
		UnassignPoint(p);
	}
}

void Quadtree::AssignPoint(Point2d* p) {
	
	Quadrant* ptr = root;
	// While the current quadrant has children, step down until we find a viable, non-divided (and possibly empty) quadrant to place point
	while (ptr->IsParent()) {
		Quadrant* next = ptr->FindChildQuadrant(p);
		ptr = next;
	}
 	ptr->AssignPoint(p, &gridLines);
}

void Quadtree::UnassignPoint(Point2d* p) {

	Quadrant* ptr = root;
	while (ptr->IsParent()) {
		Quadrant* next = ptr->FindChildQuadrant(p);
		ptr = next;
	}
	ptr->UnassignPoint(p, &gridLines);
}

std::vector<Line*>* Quadtree::GetQuadtreeGrid() {
	
	return &gridLines;
}

//void Quadtree::GetQuadtreeGrid(Quadrant* ptr, std::vector<Line*> lines) {
//	
//	if (ptr->IsParent()) {
//		// Store the lines representing the boundaries of the root quadrant
//		ptr->GetGridLines(lines);
//
//		for (int i = 0; i < ptr->getChildren().size(); i++) {
//			Quadrant* child = ptr->getChildren().at(i);
//			GetQuadtreeGrid(child, lines);
//		}
//	}
//}

std::string Quadtree::ToString() {
	Quadrant* q = root;
	int depth = 0;
	std::string output = q->ToString(depth);

	return output;
}

std::string Quadtree::Quadrant::ToString(int depth) {

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

	if (IsParent()) {
		for (int i = 0; i < children.size(); i++) {
			output << children.at(i)->ToString(depth + 1);
		}
	}

	return output.str();
}