
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

 		Node(PlanePolygon<D,T>* planePolygon) {
 			this->type = Primative;
 			this->p = planePolygon;
 			this->l = lit_Undef;
 			this->left = NULL;
 			this->right = NULL;
 			this->parentVector = new std::vector<Node<D,T>*>();
 		}

 		Node(PlanePolygon<D,T>* planePolygon, Lit active) {
 			this->type = Primative;
 			this->l = active;
 			this->p = planePolygon;
 			this->left = NULL;
 			this->right = NULL;
 			this->parentVector = new std::vector<Node<D,T>*>();
 		}

 		Node(Node<D,T>* A, Node<D,T>* B, int typeInt) {
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
 			this->l = lit_Undef;
 			this->p = NULL;
 			this->left = A;
 			this->right = B;
 			this->parentVector = new std::vector<Node<D,T>*>();
 		}

 		Node(Node<D,T>* A, Node<D,T>* B, int typeInt, Lit active) {
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
 			this->l = active;
 			this->p = NULL;
 			this->left = A;
 			this->right = B;
 			this->parentVector = new std::vector<Node<D,T>*>();
 		}

		// Determines the primative or operation.
 		nodeType type;
 		// Local var
 		Lit l;
		// If type == Primative, p will point to the primative.
 		PlanePolygon<D, T>* p;

 		Node<D,T>* left;
 		Node<D,T>* right;
 		std::vector<Node<D,T>*>* parentVector;
 		//BoundingBox<D,T> box;
 	};

#endif /* CSG_TYPES_H_ */
