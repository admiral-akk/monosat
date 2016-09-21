
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


#define MAX_INT std::numeric_limits<int>::max()
#define MIN_INT std::numeric_limits<int>::min()

 template<unsigned int D, class T = double> class CSG {

 public:
 	struct Node;
 	struct Polygon;
 	struct BoundingBox;

 	enum nodeType { Union, Intersection, Difference, Primative };

 	// Represents point containment predicate
 	struct PointContains
 	{
 		Point<D,T> p;
 		Node* solid;
 	};

 	// Map from predicate name (int) to predicate.
 	std::map<int, PointContains> pointPredicateMap;

 	// Map from boolean name (int) to node.
 	std::map<int, Node*> boolSwitchMap;
/*
 	struct Point 
 	{
 		T x, y;
 		Point(T x1, T y1) {
 			x = x1;
 			y = y1;
 		}

 		Point& operator+(const Point& a) const {
 			return Point(x+a.x,y+a.y);
 		}

 		Point& operator-(const Point& a) const {
 			return Point(x-a.x,y-a.y);
 		}

 		T operator*(const Point& a) const {
 			return x*a.y - y*a.x;
 		}

 		bool operator==(const Point& a) const {
 			return x == a.x && y == a.y;
 		}
 	};
	// A line segment is defined by two endpoints. If direction is relevant, it goes from (x1, y1) to (x2, y2).
 	struct LineSegment
 	{
 		LineSegment(T x1, T y1, T x2, T y2) {
 			start = Point(x1,y1);
 			end = Point(x2,y2);
 		}
 		Point start, end;
 	};
	// A LineSegmentNode defines the intersection of a given ray with the solid.
	struct LineSegmentNode
 	{
 		T start, end;
 		LineSegmentNode* next;
 		LineSegmentNode(T begin, T finish) {
 			start = begin;
 			end = finish;
 			next = NULL;
 		}
 	};*/
 	struct Polygon
 	{
		// The points of the convex polygon, in counter-clockwise order.
 		std::vector<Point<D,T>> vertices;
 	};

 	// BoundingBox is defined by the 4 axis-aligned lines. Left and right are the x bounds, and bottom and top are the y bounds.
 	struct BoundingBox
 	{
 		T left, right, top, bottom;

 		bool operator==(const BoundingBox& b) const {
 			return left == b.left && right == b.right && top == b.top && bottom == b.bottom;
 		}

 		BoundingBox(T leftBound, T rightBound, T lowerBound, T upperBound) {
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
 		Polygon* p;
 		Node* left;
 		Node* right;
 		Node* parent;

 	private:
 		// We hide these so that we don't have to overwrite them when the Active bool is flipped.
		// Maps from predicate name to boolean.
 		std::map<int,bool> pointContainment;
 		// Maps from predicate name to line segment.
 		// std::map<int,LineSegmentNode*> pointContainment;
 		BoundingBox box;

 	public:
 		T getBoundingBoxLeft() {
 			return active ? box.left : MAX_INT;
 		}
 		T getBoundingBoxRight() {
 			return active ? box.right : MIN_INT;
 		}
 		T getBoundingBoxTop() {
 			return active ? box.top : MIN_INT;
 		}
 		T getBoundingBoxBottom() {
 			return active ? box.bottom : MAX_INT;
 		}
 		T getBoundingBox() {
 			return active ? box : BoundingBox(MAX_INT,MIN_INT,MAX_INT,MIN_INT);
 		}
 		void setBoundingBox(BoundingBox boundingBox) {
 			box = boundingBox;
 		}
 		void setBoundingBox(T leftBound, T rightBound, T lowerBound, T upperBound) {
 			box = BoundingBox(leftBound,rightBound,lowerBound,upperBound);
 		}
 		bool checkIfPredicateContained(T index) {
 			return pointContainment.count(index);
 		}
 		bool getPointPredicate(T index) {
 			return active ? pointContainment[index] : false;
 		}
 		void setPointPredicate(T index, bool val) {
 			if (pointContainment.count(index)) 
 				pointContainment.erase(index);
 			pointContainment.emplace(index,val);
 		}

 	};

 public:

 private:
 	
	//
	// Point Containment
	//

	//
	// Initialization
	//

 	bool leftTurn(Point<2,T> p1, Point<2,T> p2, Point<2,T> p3) {
 		return (p2-p1)*(p3-p1) >= 0;
 	}

 	bool contains(Point<2,T> p, Polygon* shape) {
 		for (int i = 0; i < shape.size(); i++) {
 			if (!leftTurn(shape.vertices[i],shape.vertices[(i+1) % shape.size()],p))
 				return false;	
 		}
 		return true;
 	}

 	bool contains(Point<2,T> p, BoundingBox box) {
 		return p.x >= box.left && p.x <= box.right && p.y >= box.bottom && p.y <= box.top;
 	}

 	void initializeContainsPoint(int index, Node* node) {
 		Point<2,T> p = pointPredicateMap[index].p;

 		// If it's outside of the bounding box, the query is simple.
 		if (!contains(p,node->getBoundingBox())) {
 			node->setPointPredicate(index, false);
 			return;
 		}


 		switch (node->type) {
 			case Union:
 			if (!node->left->checkIfPredicateContained())
 				initializeContainsPoint(index,node->left);
 			if (node->left->getPointPredicate(index)) {
 				node->setPointPredicate(index, true);
 				return;
 			}
 			if (!node->right->checkIfPredicateContained())
 				initializeContainsPoint(index,node->right);
 			if (node->right->getPointPredicate(index)) {
 				node->setPointPredicate(index, true);
 				return;
 			}
 			node->setPointPredicate(index, false);
 			return;

 			case Intersection:

 			if (!node->left->checkIfPredicateContained())
 				initializeContainsPoint(index,node->left);
 			if (!node->left->getPointPredicate(index)) {
 				node->setPointPredicate(index, false);
 				return;
 			}
 			if (!node->right->checkIfPredicateContained())
 				initializeContainsPoint(index,node->right);
 			if (!node->right->getPointPredicate(index)) {
 				node->setPointPredicate(index, false);
 				return;
 			}
 			node->setPointPredicate(index, true);
 			return;

 			case Difference:

 			if (!node->left->checkIfPredicateContained())
 				initializeContainsPoint(index,node->left);
 			if (!(node->left->getPointPredicate(index))) {
 				node->setPointPredicate(index, false);
 				return;
 			}
 			if (!node->right->checkIfPredicateContained())
 				initializeContainsPoint(index,node->right);
 			if (node->right->getPointPredicate(index)) {
 				node->setPointPredicate(index, false);
 				return;
 			}
 			node->setPointPredicate(index, true);
 			return;

 			case Primative:
 			if (!node->checkIfPredicateContained())
 				node->setPointPredicate(index, contains(p,node->p));
 			return;

 			default:
 			perror("(shape->type, Point): HOW DID YOU GET HERE?!\n");
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
 		bool old = node->getPointPredicate(index);
 		initializeContainsPoint(index,node);
 		if (old == node->getPointPredicate(index)) 
 			return;
 		updateParentPoint(index, node->parent);
 	} 	

	//
	// Bounding Box
	//

	//
	// Initialization
	//

 	void updateNodeBoundingBox(Node* node) {
 		switch (node->type) {
 			case Primative:
 			node->setBoundingBox(initializeBoundingBox(node->p));
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
 			perror("HOW DID YOU GET HERE?!\n");
 			break;
 		}
 	}

 	BoundingBox initializeBoundingBox(Polygon* p) {
 		T leftBound, rightBound, upperBound, lowerBound;
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
 		T leftBound, rightBound, upperBound, lowerBound;
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
 		T leftBound, rightBound, upperBound, lowerBound;
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
 		if (node->type != Primative) {
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
 		BoundingBox old = node->getBoundingBox();
 		updateNodeBoundingBox(node);
 		if (old == node->getBoundingBox()) 
 			return;
 		updateParentBoundingBox(node->parent);
 	}

/*

	//
	// Line Segment Containment
	//

	T dotProduct(Point p1, Point p2) {
		return p1.x*p2.x + p1.y+p2.y;
	}

	// Followed from here: http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
	// Parameterize r1 with t over [0,1].
	// Returns the maximum and minimum t values which intersect r2.

	LineSegmentNode* intersects(LineSegment r1, LineSegment r2) {
		Point p = r1.start;
		Point r = r1.end - r1.start;
		Point q = r2.start;
		Point s = r2.end - r2.start;
		Point qp = q-p;
		T rs = r*s;
		T rLength = dotProduct(r,r);
		LineSegmentNode* answer;
		// Parallel
		if (rs == 0) {
			// Colinear
			if (qp*r == 0) {
				t0 = dotProduct(qp,r)/rLength;
				t1 = t0+dotProduct(s,r)/rLength; 
				if (t0 <= 1 && t1 >= 0) {
					answer = new LineSegmentNode(std::max(t0,0),std::min(t1,1));
					return answer;
				}
			}
			// Not colinear, or colinear and disjoint.
			return NULL;
		} 
		T t = qp*s/rs;
		T u = qp*r/rs;
		// Intersection between the end points.
		if (0 <= t && t <= 1 && 0 <= u && u <= 1) {
			answer = new LineSegmentNode(t,t);
			return answer;
		}
		return NULL;
	}

	LineSegmentNode* intersects(LineSegment r, Primative p) {
		LineSegmentNode* 
 		for (int i = 0; i < shape.size(); i++) {
 			LineSegment edge = LineSegment(shape.vertices[i].x, shape.vertices[i].y, shape.vertices[(i+1) % shape.size()].x, shape.vertices[(i+1) % shape.size()].y);
 			LineSegmentNode* intersection = intersects(r, edge);
 			if (intersection) {

 			}
 		}
	}


	// Followed from here: http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

	bool intersects(LineSegment r1, LineSegment r2) {
		Point p = r1.start;
		Point r = r1.end - r1.start;
		Point q = r2.start;
		Point s = r2.end - r2.start;
		Point qp = q-p;
		T rs = r*s;
		T rLength = dotProduct(r,r);
		// Parallel
		if (rs == 0) {
			// Colinear
			if (qp*r == 0) {
				t0 = dotProduct(qp,r)/rLength;
				t1 = t0+dotProduct(s,r)/rLength; 
				if (t0 <= 1 && t1 >= 0) {
					return true;
				}
			}
			// Not colinear, or colinear and disjoint.
			return false;
		} 
		T t = qp*s/rs;
		T u = qp*r/rs;
		// Intersection between the end points.
		if (0 <= t && t <= 1 && 0 <= u && u <= 1) 
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

	// TODO: FIX MEMORY LEAKS!
	LineSegmentNode* intersects(LineSegment r, Node* shape) {
		LineSegmentNode* raySegments = NULL;
		LineSegmentNode* left = NULL;
		LineSegmentNode* right = NULL;
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
*/
#endif CSG_H_
