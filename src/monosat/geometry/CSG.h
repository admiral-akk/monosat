
#ifndef CSG_H_
#define CSG_H_
#include <vector>
#include <map>
#include "CSGTypes.h"
#include <stdio.h>
#include <limits.h>

/**
 * Stores our solid, and answers predicates across it.
 */

template<unsigned int D, class T>
 class NodePointer;

 template<unsigned int D, class T = int> class CSG {


 private:
 	/*
	//
	// Bounding Box
	//

	//
	// Initialization
	//

 	void initializeBoundingBox(int rootIndex) {
 		Node<D,T> root = shapes[rootIndex];
 		if (root.type != Primative) {
 			initializeBoundingBox(root.leftNode());
 			initializeBoundingBox(root.rightNode());
 		}
 		root.box.update();
 	}

	//
	// Updates
	//

 	void updateParentBoundingBox(int nodeIndex) {
 		Node<D,T> node = shapes[nodeIndex];
 		if (!shapes[nodeIndex].box.update()) 
 			return;
 		for (auto parentNode : shapes[nodeIndex].parentVector)
 			updateParentBoundingBox(shapes[parentNode]);
 	}
	*/

 public:
 	std::vector<Node<D,T>*> shapes;

 	CSG() {
 	}

 	//
 	// Update Boolean
 	// 
 	Node<D,T>* getNode(int index) {
 		return shapes[index];
 	}

 	void updateBoolean(bool value, int shapeIndex) {
 		shapes[shapeIndex]->active = value;
 	}
};
#endif /* CSG_H_ */
