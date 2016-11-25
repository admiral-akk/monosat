
#ifndef CSG_TYPES_H_
#define CSG_TYPES_H_

enum nodeType { Union, Intersection, Difference, Primative };

#include "monosat/core/SolverTypes.h"
#include "Polygon.h"
#include <vector>
#include "bounds/BoundingBox.h"

template<unsigned int D, class T> 
struct Node
{

public:
	Node() {
		this->type = Primative;
		this->active = false;
		this->p = NULL;
		this->name = -1;
		this->left = -1;
		this->right = -1;
	}

	Node(const Node<D,T>& n) {
		this->type = n.type;
		this->active = n.active;
		this->p = n.p;
		this->name = n.name;
		this->left = n.left;
		this->right = n.right;
	}

	Node(Node<D,T>& n) {
		this->type = n.type;
		this->active = n.active;
		this->p = n.p;
		this->name = n.name;
		this->left = n.left;
		this->right = n.right;
	}

	Node(PlanePolygon<D,T>* planePolygon, bool active, int name) {
		this->type = Primative;
		this->active = active;
		this->p = planePolygon;
		this->name = name;
		this->left = -1;
		this->right = -1;
	}

	Node(int A, int B, int typeInt, bool active, int name) {
		nodeType type;
		switch (typeInt) {
		case 0:
			type = Union;
			break;
		case 1:
			type = Intersection;
			break;
		case 2:
			type = Difference;
			break;
		default:
			break;
		}
		this->type = type;
		this->active = active;
		this->name = name;
		this->p = NULL;
		this->left = A;
		this->right = B;
	}

	// Determines the primative or operation.
	nodeType type;
	// Needed to get variable from theory
	int name;
	// Determines if node is active
	bool active;
	// If type == Primative, p will point to the primative.
	PlanePolygon<D, T>* p;

	int left, right;
	vec<int> parentVector;
	//BoundingBox<D,T> box;
};

#endif /* CSG_TYPES_H_ */
