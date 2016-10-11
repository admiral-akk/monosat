
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

 template<unsigned int D, class T = double> class CSG {


 private:

 	// Map from boolean name to node.
 	std::map<Var, Node<D,T>*> boolToNode;

	//
	// Bounding Box
	//

	//
	// Initialization
	//

 	void initializeBoundingBox(Node<D,T>* root) {
 		if (root->type != Primative) {
 			initializeBoundingBox(root->left);
 			initializeBoundingBox(root->right);
 		}
 		root->box.update();
 	}

	//
	// Updates
	//

 	void updateParentBoundingBox(Node<D,T>* node) {
 		if (!node)
 			return;
 		if (!node->box.update()) 
 			return;
 		updateParentBoundingBox(node->parent);
 	}

 public:

 	//
 	// Update Boolean
 	// 

 	void updateBoolean(Lit value) {
 		boolToNode[var(value)]->conditional = value;
 		updateParentBoundingBox(boolToNode[var(value)]);
 	}
};
#endif /* CSG_H_ */
