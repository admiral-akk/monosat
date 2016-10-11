
#ifndef CSG_TYPES_H_
#define CSG_TYPES_H_
 
enum nodeType { Union, Intersection, Difference, Primative };

#include "monosat/core/SolverTypes.h"
#include "Polygon.h"
#include "bounds/BoundingBox.h"


template<unsigned int D, class T> 
 	struct Node 
 	{

 	public:
		// Determines the primative or operation.
 		nodeType type;
 		// Undef if no conditional applies
 		Lit conditional;
		// If it's a leaf, p will point to the primative.
 		Polygon<D, T>* p;
 		Node* left;
 		Node* right;
 		Node* parent;
 		BoundingBox<D,T> box;

 	};

#endif /* CSG_TYPES_H_ */
