/****************************************************************************************[Solver.h]
 The MIT License (MIT)

 Copyright (c) 2014, Sam Bayless

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **************************************************************************************************/

#ifndef BOUNDINGBOX_H_
#define BOUNDINGBOX_H_

#include "BoundingVolume.h"
#include "../Polygon.h"
#include "../ConvexPolygon.h"
#include "../CSGTypes.h"
#include <vector>

//axis-aligned bounding box

template<unsigned int D, class T>
class AbstractBoundingBox: public BoundingVolume<D, T> {
protected:
	Point<D, T> max_point;
	Point<D, T> min_point;
	//These are important to pre-calculate for the case where we are using expensive arbitrary precision arithmetic.
	Point<D, T> top_left_point;
	Point<D, T> bottom_right_point;

public:
	AbstractBoundingBox() {
		
	}
	virtual ~AbstractBoundingBox() {
		
	}
	ShapeType getType() {
		return BOUNDING_BOX;
	}

	/*
	bool intersects(Shape<D, T> & s, bool inclusive = true) {
		
		if (s.getType() == LINE_SEGMENT) {
			if (D == 2) {
				return intersectsLine2d((LineSegment<2, T>&) s, inclusive);
			} else {
				assert(false);
			}
		} else if (s.getType() == POLYGON || s.getType() == CONVEX_POLYGON) {
			Polygon<D, T> & poly = (Polygon<D, T> &) s;
			if (poly.hasBound()) {
				return intersects(*poly.getBound(), inclusive);
			}
			return true;
		} else if (s.getType() == BOUNDING_BOX) {
			AbstractBoundingBox<D, T> & B = (AbstractBoundingBox<D, T> &) s;
			
			for (int i = 0; i < D; i++) {
				if (max_point[i] < B.min_point[i] || min_point[i] > B.max_point[i])
					return false;
			}
			
			return true;
		} else if (s.getType() == BOUNDING_SPHERE) {
			
			//T sqdist = squared_distance()
			
		}
		
		return true;
	}
	
private:
	
	bool intersectsLine2d(LineSegment<2, T> & line, bool inclusive) {
		if (line.a == line.b) {
			//this is really a point - and it will cause problems below, because the line has no sides.
			return this->contains(line.a, inclusive);
		}
		
		//check if the corners of the box are all on the same side of the line segment
		int side = (line.whichSide(max_point));
		if (side == 0 && inclusive) {
			return true;
		}
		int last_side = side;
		side = (line.whichSide(min_point));
		if (side == 0) {
			if (inclusive)
				return true;
		} else {
			if (last_side != 0 && side != last_side) {
				return true;
			}
			last_side = side;
		}
		side = (line.whichSide(top_left_point));
		if (side == 0) {
			if (inclusive)
				return true;
		} else {
			if (last_side != 0 && side != last_side) {
				return true;
			}
			last_side = side;
		}
		side = (line.whichSide(bottom_right_point));
		if (side == 0) {
			if (inclusive)
				return true;
		} else {
			if (last_side != 0 && side != last_side) {
				return true;
			}
		}
		//is this really the only test we need? 
		// Nope. If it were a ray, yes. But you need to show that a part lies in the box.
		
		return false;
	}
	bool intersectsConvex2d(ConvexPolygon<2, T> & polygon, bool inclusive) {
		
		return false;
	}*/
};
/*
template<unsigned int D, class T, class Bound>
class BoundingBox: public AbstractBoundingBox<D, T> {
	Bound & toBound;
public:
	BoundingBox(Bound & toBound) :
			toBound(toBound) {
		
	}
};*/

template<unsigned int D, class T>
class Node;

template<unsigned int D, class T>
class NodePointer;

template<unsigned int D, class T>
class BoundingBox {

	private:
	std::vector<Node<D,T>>* nodeVector;
	int nodeIndex;
	Point<D, T> max_point;
	Point<D, T> min_point;

public:
	T getMin(int i) {
		Node<D,T> node = nodeVector[nodeIndex];
		if (node.conditional == lit_Undef || sign(node.conditional))
			return this->min_point[i];
		else
			return numeric<T>::infinity();
	}

	T getMax(int i) {
		Node<D,T> node = nodeVector[nodeIndex];
		if (node.conditional == lit_Undef || sign(node.conditional))
			return this->max_point[i];
		else
			return -numeric<T>::infinity();
	}

	BoundingBox() {	}

	BoundingBox(std::vector<Node<D,T>>* nodeVec, int owner) {
		this->nodeIndex = owner;
		this->nodeVector = nodeVec;
	}

	ShapeType getType() {
		return BOUNDING_BOX;
	}
	
	// Returns true if update causes changes
	bool update() {
		Node<D,T> nodeVal = nodeVector[nodeIndex];
		Node<D,T>* node = &nodeVal;
		bool changes = false;
		nodeType type = node->type;

		T newMin, newMax;
		switch (type) {
			case Primative: {
				Polygon<D, T>* poly = node->p;
				for (auto & p : *poly) {
					for (int i = 0; i < D; i++) {
						if (p[i] > this->max_point[i])
							this->max_point[i] = p[i];
						if (p[i] < this->min_point[i])
							this->min_point[i] = p[i];
					}
				}
				break;
			}
			case Union:
				for (int i = 0; i < D; i++) {
					newMax = (node->left->box.getMax(i) > node->right->box.getMax(i)) ? node->left->box.getMax(i) : node->right->box.getMax(i);
					newMin = (node->left->box.getMin(i) > node->right->box.getMin(i)) ? node->right->box.getMin(i) : node->left->box.getMin(i);
					if (this->max_point[i] != newMax || this->min_point[i] != newMin)
						changes = true;
					this->max_point[i] = newMax;
					this->min_point[i] = newMin;
				}
				break;
			case Intersection:
				for (int i = 0; i < D; i++) {
					if (node->left->box.getMax(i) < node->left->box.getMin(i) || node->right->box.getMax(i) < node->right->box.getMin(i)) {
						for (int j = 0; j < D; j++) {
							if (this->max_point[i] != -numeric<T>::infinity() || this->min_point[i] != numeric<T>::infinity())
								changes = true;
							this->max_point[i] = -numeric<T>::infinity();
							this->min_point[i] = numeric<T>::infinity();
						}
						break;
					}

					newMax = (node->left->box.getMax(i) < node->right->box.getMax(i)) ? node->left->box.getMax(i) : node->right->box.getMax(i);
					newMin = (node->left->box.getMin(i) < node->right->box.getMin(i)) ? node->right->box.getMin(i) : node->left->box.getMin(i);
					if (this->max_point[i] != newMax || this->min_point[i] != newMin)
						changes = true;
					this->max_point[i] = newMax;
					this->min_point[i] = newMin;
				}
				break;
			case Difference:
				for (int i = 0; i < D; i++) {
					newMax = node->left->box.getMax(i);
					newMin = node->left->box.getMin(i);

					if (this->max_point[i] != newMax || this->min_point[i] != newMin) 
						changes = true;
					this->max_point[i] = newMax;
					this->min_point[i] = newMin;
				}
				break;
			default:
				break;
		}
		return changes;
	}
	bool contains(const Point<D, T> & point, bool inclusive = true) {
		Node<D,T> node = nodeVector[nodeIndex];
		if (node.conditional == lit_Undef)
			return false;
		if (inclusive) {
			for (int i = 0; i < D; i++) {
				if (point[i] > this->max_point[i])
					return false;
				if (point[i] < this->min_point[i])
					return false;
			}
		} else {
			for (int i = 0; i < D; i++) {
				if (point[i] >= this->max_point[i])
					return false;
				if (point[i] <= this->min_point[i])
					return false;
			}
		}
		return true;
	}
	/*
	bool dbg_uptodate() {
#ifndef NDEBUG
		//std::vector<Point<D,T>>& vertices  = toBound.getVertices();
		Point<D, T> dbg_max;
		Point<D, T> dbg_min;
		for (int i = 0; i < D; i++) {
			dbg_max[i] = -numeric<T>::infinity();
			dbg_min[i] = numeric<T>::infinity();
		}
		for (auto & p : toBound) {
			for (int i = 0; i < D; i++) {
				if (p[i] > dbg_max[i])
					dbg_max[i] = p[i];
				if (p[i] < dbg_min[i])
					dbg_min[i] = p[i];
			}
		}
		
		assert(this->max_point == dbg_max);
		assert(this->min_point == dbg_min);
#endif
		return true;
	}*/
};

#endif /* BOUNDINGBOX_H_ */
