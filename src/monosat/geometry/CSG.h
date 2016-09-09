
#ifndef CSG_H_
#define CSG_H_
#include <vector>
#include <map>
#include "GeometryTypes.h"
#include <stdio.h>
#include <limits.h>

/**
 * Stores our solid, and answers predicates across it.
 */

 struct Rational 
 {
 	long p, q;
 	Rational(long p1, long q1) {
 		p = p1;
 		q = q1;
 	}

 	Rational& operator*(const Rational& a) const {
 		return Rational(p*a.p,q*a.q);
 	}

 	Rational& operator+(const Rational& a) const {
 		return Rational(p*a.q+q*a.p,q*a.q);
 	}

 	Rational& operator-(const Rational& a) const {
 		return Rational(p*a.q+q*a.p,q*a.q);
 	}

 	bool operator>(const Rational& a) const {
 		return p*a.q > q*a.p;
 	}
 };

 template<unsigned int D, class T=Rational>
 enum nodeType { Union, Intersection, Difference, Primative };
 class CSG {

 public:

 	// Represents point containment predicate
 	struct PointContains
 	{
 		Point p;
 		Node* solid;
 	};

 	// Map from predicate name (int) to predicate.
 	std::map<int, PointContains> pointPredicateMap;

 	struct Point 
 	{
 		int x, y;
 		Point(int x1, int y1) {
 			x = x1;
 			y = y1;
 		}

 		Point& operator+(const Point& a) const {
 			return Point(x+a.x,y+a.y);
 		}

 		Point& operator-(const Point& a) const {
 			return Point(x-a.x,y-a.y);
 		}

 		long operator*(const Point& a) const {
 			return x*a.y - y*a.x;
 		}

 		bool operator==(const Point& a) const {
 			return x == a.x && y == a.y;
 		}
 	};
	// A line segment is defined by two endpoints. If direction is relevant, it goes from (x1, y1) to (x2, y2).
 	struct LineSegment
 	{
 		LineSegment(int x1, int y1, int x2, int y2) {
 			start = Point(x1,y1);
 			end = Point(x2,y2);
 		}
 		Point start, end;
 	};
	// A segment node is defined by two rational endpoints, r1/q1 and r2/q2.
 	struct SegmentNode
 	{
 		T start, end;
 		LineSegment(long r1, long q1, long r2, long q2) {
 			start = T(r1,q1);
 			end = T(r2,q2);
 		}
 		SegmentNode* next;
 	};

 	struct Primative
 	{
		// The points of the convex primative, in counter-clockwise order.
 		std::vector<Point> vertices;
 	};

 	// BoundingBox is defined by the 4 axis-aligned lines. Left and right are the x bounds, and bottom and top are the y bounds.
 	struct BoundingBox
 	{
 		int left, right, top, bottom;

 		bool operator==(const BoundingBox& b) const {
 			return left == b.left && right == b.right && top == b.top && bottom == b.bottom;
 		}

 		BoundingBox(int leftBound, int rightBound, int lowerBound, int upperBound) {
 			left = leftBound;
 			right = rightBound;
 			bottom = lowerBound;
 			top = upperBound;
 		}
 	};

 	struct Node 
 	{

 	public:
		// Determines the primative or operation.
 		nodeType type;
		// If false, this node is simply the null set.
 		bool active;
		// If it's a leaf, p will point to the primative.
 		Primative* p;
 		Node* left;
 		Node* right;
 		Node* parent;

 	private:
		// Maps from predicate name to boolean.
 		std::map<int,bool> pointContainment;
 		BoundingBox box;

 	public:
 		int getBoundingBoxLeft() {
 			return active ? box.left : MAX_INT;
 		}
 		int getBoundingBoxRight() {
 			return active ? box.right : MIN_INT;
 		}
 		int getBoundingBoxTop() {
 			return active ? box.top : MIN_INT;
 		}
 		int getBoundingBoxBottom() {
 			return active ? box.bottom : MAX_INT;
 		}
 		int getBoundingBox() {
 			return active ? box : BoundingBox(MAX_INT,MIN_INT,MAX_INT,MIN_INT);
 		}
 		void setBoundingBox(BoundingBox boundingBox) {
 			box = boundingBox;
 		}
 		void setBoundingBox(int leftBound, int rightBound, int lowerBound, int upperBound) {
 			box = BoundingBox(leftBound,rightBound,lowerBound,upperBound);
 		}
 		bool checkIfPredicateContained(int index) {
 			return pointContainment.count(index);
 		}
 		bool getPredicate(int index) {
 			return active ? pointContainment[index] : false;
 		}
 		void setPredicate(int index, bool val) {
 			if (pointContainment.count(index)) 
 				pointContainment.erase(index)
 			pointContainment.emplace(index,val);
 		}

 	};

 public:

 private:

	//
	// Bounding Box
	//

	//
	// Initialization
	//

 	void updateNodeBoundingBox(Node* node) {
 		switch (shape->type) {
 			case Primative:
 			node->setBoundingBox(initializeBoundingBox(node->primative));
 			break;

 			case Union:
 			node->setBoundingBox(initializeUnionBoundingBox(node->left, node->right));
 			break;

 			case Difference:
 			node->setBoundingBox(initializeDifferenceBoundingBox(node->left, node->right));
 			break;

 			case Intersection:
 			node->setBoundingBox(initializeIntersectBoundingBox(node->left, node->right));
 			break;

 			default:
 			perror("HOW DID YOU GET HERE?!\n")
 			break;
 		}
 	}

 	BoundingBox initializeBoundingBox(Primative* p) {
 		int leftBound, rightBound, upperBound, lowerBound;
 		leftBound = MAX_INT;
 		rightBound = MIN_INT;
 		lowerBound = MAX_INT;
 		upperBound = MIN_INT;
 		for (int i = 0; i < p->vertices.size(); i++) {
 			leftBound = std::min(leftBound, p->vertices[i].x);
 			rightBound = std::max(rightBound, p->vertices[i].x);
 			lowerBound = std::min(lowerBound, p->vertices[i].y);
 			upperBound = std::max(upperBound, p->vertices[i].y);
 		}
 		return BoundingBox(leftBound,rightBound,lowerBound,upperBound);
 	}

 	BoundingBox initializeUnionBoundingBox(Node* left, Node* right) {
 		int leftBound, rightBound, upperBound, lowerBound;
 		leftBound = std::min(left->getBoundingBoxLeft(),right->getBoundingBoxLeft());
 		rightBound = std::max(left->getBoundingBoxRight(),right->getBoundingBoxRight());
 		lowerBound = std::min(left->getBoundingBoxBottom(),right->bgetBoundingBoxBottom());
 		upperBound = std::max(left->getBoundingBoxTop(),right->getBoundingBoxTop());
 		return BoundingBox(leftBound,rightBound,lowerBound,upperBound);
 	}

	// Can't conclude anything interesting about the bounding box in the case of set difference. Just return the left bounding box.
 	BoundingBox initializeDifferenceBoundingBox(Node* left, Node* right) {
 		return BoundingBox(left->box);
 	}

 	BoundingBox initializeIntersectBoundingBox(Node* left, Node* right) {
 		int leftBound, rightBound, upperBound, lowerBound;
 		leftBound = std::max(left->getBoundingBoxLeft(),right->getBoundingBoxLeft());
 		rightBound = std::min(left->getBoundingBoxRight(),right->getBoundingBoxRight());
 		lowerBound = std::max(left->getBoundingBoxBottom(),right->getBoundingBoxBottom());
 		upperBound = std::min(left->getBoundingBoxTop(),right->getBoundingBoxTop());

		// If the bounding box is empty. This deals with things like unions.
 		if (leftBound > rightBound || lowerBound > upperBound) 
 			return BoundingBox(MAX_INT,MIN_INT,MAX_INT,MIN_INT);

 		return BoundingBox(leftBound,rightBound,lowerBound,upperBound);

 	}
 	void initializeBoundingBox(Node* node) {
 		if (shape->type != Primative) {
 			initializeBoundingBox(node->left);
 			initializeBoundingBox(node->right);
 		}
 		updateNodeBoundingBox(node);
 	}

	//
	// Updates
	//

 	void updateParentBoundingBox(Node* node) {
 		if (!node)
 			return;
 		BoundingBox old = node->box;
 		updateNodeBoundingBox(node);
 		if (old == node->getBoundingBox()) 
 			return;
 		updateParentBoundingBox(node->parent);
 	}

	//
	// Point Containment
	//

	//
	// Initialization
	//

 	bool leftTurn(Point p1, Point p2, Point p3) {
 		return (p2-p1)*(p3-p1) >= 0;
 	}

 	bool contains(Point p, Primative* shape) {
 		for (int i = 0; i < shape.size(); i++) {
 			if (!leftTurn(shape.vertices[i],shape.vertices[(i+1) % shape.size(),p]))
 				return false;	
 		}
 		return true;
 	}

 	bool contains(Point p, BoundingBox box) {
 		return p.x >= box.left && p.x <= box.right && p.y >= box.bottom && p.y <= box.top;
 	}

 	/*
 	bool contains(Point p, Node* shape) {
 		if (!contains(p,shape->box)) 
 			return false;
 		switch (shape->type) {
 			case Union:
 			if (contains(p,shape->left))
 				return true;
 			return contains(p,shape->right);

 			case Intersection:
 			if (!contains(p,shape->left))
 				return false;
 			return !contains(p,shape->right);

 			case Difference:
 			if (!contains(p,shape->left))
 				return false;
 			return !contains(p,shape->right);

 			case Primative:
 			return contains(p,shape->primative);

 			default:
 			perror("(shape->type, Point): HOW DID YOU GET HERE?!\n")
 			break;
 		}
 	}
 	*/

 	void initializeContainsPoint(int index, Node* node) {
 		Point p = pointPredicateMap[index].p;

 		// If it's outside of the bounding box, the query is simple.
 		if (!contains(p,node->getBoundingBox())) {
 			node->setPredicate(index, false);
 			return;
 		}


 		switch (node->type) {
 			case Union:
 			initializeContainsPoint(index,node->left);
 			if (node->getPredicate(index)) {
 				node->setPredicate(index, true);

 				return;
 			}
 			initializeContainsPoint(index,node->right);
 			if (node->getPredicate(index)) {
 				node->setPredicate(index, true);
 				return;
 			}
 			node->setPredicate(index, false);
 			return;

 			case Intersection:

 			initializeContainsPoint(index,node->left);
 			if (node->getPredicate(index)) {
 				node->setPredicate(index, false);
 				return;
 			}
 			initializeContainsPoint(index,node->right);
 			if (node->getPredicate(index)) {
 				node->setPredicate(index, false);
 				return;
 			}
 			node->setPredicate(index, true);
 			return;

 			case Difference:

 			initializeContainsPoint(index,node->left);
 			if (!(node->getPredicate(index))) {
 				node->setPredicate(index, false);
 				return;
 			}
 			initializeContainsPoint(index,node->right);
 			if (node->getPredicate(index)) {
 				node->setPredicate(index, false);
 				return;
 			}
 			node->setPredicate(index, true);
 			return;

 			case Primative:
 			node->setPredicate(index, contains(p,node->primative));
 			return;

 			default:
 			perror("(shape->type, Point): HOW DID YOU GET HERE?!\n")
 			break;
 		}
 	}

 	void initializeContainsPoint(int index) {
 		initializeContainsPoint(index,pointPredicateMap[index].node);
 	}

	//
	// Updates
	//

 	void updateParentPoint(int index, Node* node) {
 		if (!node || !(node->checkIfPredicateContained(index)))
 			return;
 		bool old = node->getPredicate(index);
 		initializeContainsPoint(index,node);
 		if (old == node->getPredicate(index)) 
 			return;
 		updateParentBoundingBox(node->parent);
 	}


	//
	// Line Segment Containment
	//
	/*

	long dotProduct(Point p1, Point p2) {
		return p1.x*p2.x + p1.y+p2.y;
	}

	// Followed from here: http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
	bool intersects(LineSegment r1, LineSegment r2) {
		Point p = r1.start;
		Point r = r1.end - r1.start;
		Point q = r2.start;
		Point s = r2.end - r2.start;
		Point qp = q-p;
		long rs = r*s;
		long rLength = dotProduct(r,r);
		// Parallel
		if (rs == 0) {
			// Colinear
			if (qp*r == 0) {
				t0 = dotProduct(qp,r);
				t1 = t0+dotProduct(s,r); 
				if (t0 <= rLength && t1 >= 0)
					return true;
			}
			// Not colinear, or colinear and disjoint.
			return false;
		} 
		long t = qp*s;
		long u = qp*r;
		// Intersection between the end points.
		if (0 <= t && t <= rs && 0 <= u && u <= rs) 
			return true;
		return false;
	}

	bool intersects(LineSegment r, BoundingBox b) {
		LineSegment boxEdge = LineSegment(b.x,b.y,b.x+b.xSize,b.y);
		if (intersects(r,boxEdge))
			return true;
		boxEdge = LineSegment(b.x+b.xSize,b.y,b.x+b.xSize,b.y+b.ySize);
		if (intersects(r,boxEdge))
			return true;
		boxEdge = LineSegment(b.x+b.xSize,b.y+b.ySize,b.x,b.y+b.ySize);
		if (intersects(r,boxEdge))
			return true;
		boxEdge = LineSegment(b.x,b.y,b.x,b.y+b.ySize);
		if (intersects(r,boxEdge))
			return true;
		Point endpoint = Point(r.x1,r.y1);
		if (contains(endpoint,b))
			return true;
		return false;
	}

	SegmentNode* intersects(LineSegment r, Primative* p) {

	}

	SegmentNode* union(SegmentNode* left, SegmentNode* right) {

		if (!left && !right) 
			return NULL;
		if (!left)
			return union(right, left);
		if (!right) {

		}

	}

	// TODO: FIX MEMORY LEAKS!
	SegmentNode* intersects(LineSegment r, Node* shape) {
		SegmentNode* raySegments = NULL;
		SegmentNode* left = NULL;
		SegmentNode* right = NULL;
		if (!intersects(p,shape->box)) 
			return raySegments;
		if (shape->type == Primative) 
			return intersects(r,shape->primative);
		left = intersects(r,shape->left);
		right = intersects(r,shape->right);
		switch (shape->type) {
			case Union:
				return union(left,right);
			case Intersection:
				return intersect(left,right);
			case Difference:
				return difference(left,right);
			default:
				perror("(shape->type, LineSegment): HOW DID YOU GET HERE?!\n")
				break;
		}
	}*/
#endif CSG_H_
