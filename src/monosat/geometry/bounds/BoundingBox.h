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

#include "CSG.h"
#include "BoundingVolume.h"
#include "../Polygon.h"
#include "../ConvexPolygon.h"
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
class BoundingBox<D, T> : public AbstractBoundingBox<D, T> {
private:
	Node* node;

public:
	T getMin(int i) {
		if (owner->conditional == Lit_Undef || sign(owner->conditional))
			return this->min_point[i];
		else
			return numeric<T>::infinity();
	}

	T getMax(int i) {
		if (owner->conditional == Lit_Undef || sign(owner->conditional))
			return this->max_point[i];
		else
			return -numeric<T>::infinity();
	}

	BoundingBox(Node* owner) {
		this.node = owner;
	}
	ShapeType getType() {
		return BOUNDING_BOX;
	}
	
	// Returns true if update causes changes
	bool update() {
		bool changes = false;
		NodeType type = node->type;
		if (type != Primative) {
			BoundingBox<D, T> A = node->left.box; 
			BoundingBox<D, T> B = node->right.box; 
		}

		T newMin, newMax;
		switch (type) {
			case Primative:
				Polygon<D, T> p = node->p
				for (auto & p : toBound) {
					for (int i = 0; i < D; i++) {
						if (p[i] > this->max_point[i])
							this->max_point[i] = p[i];
						if (p[i] < this->min_point[i])
							this->min_point[i] = p[i];
					}
				}
				break;
			case Union:
				for (int i = 0; i < D; i++) {
					newMax = (A.getMax(i) > B.getMax(i)) ? A.getMax(i) : B.getMax(i);
					newMin = (A.getMin(i) > B.getMin(i)) ? B.getMin(i) : A.getMin(i);
					if (max_point[i] != newMax || min_point[i] != newMin)
						changes = true;
					max_point[i] = newMax;
					min_point[i] = newMin;
				}
				break;
			case Intersect:
				for (int i = 0; i < D; i++) {
					if (A.getMax(i) < A.getMin(i) || B.getMax(i) < B.getMin(i)) {
						for (int j = 0; j < D; j++) {
							if (max_point[i] != -numeric<T>::infinity() || min_point[i] != numeric<T>::infinity())
								changes = true;
							max_point[i] = -numeric<T>::infinity();
							min_point[i] = numeric<T>::infinity();
						}
						break;
					}

					newMax = (A.getMax(i) < B.getMax(i)) ? A.getMax(i) : B.getMax(i);
					newMin = (A.getMin(i) < B.getMin(i)) ? B.getMin(i) : A.getMin(i);
					if (max_point[i] != newMax || min_point[i] != newMin)
						changes = true;
					max_point[i] = newMax;
					min_point[i] = newMin;
				}
				break;
			case Difference:
				for (int i = 0; i < D; i++) {
					newMax = A.getMax();
					newMin = A.getMin();

					if (max_point[i] != newMax || min_point[i] != newMin) 
						changes = true;
					max_point[i] = newMax;
					min_point[i] = newMin;
				}
				break;
			default:
				break;
		}
		return changes;
	}
	bool contains(const Point<D, T> & point, bool inclusive = true) {
		if (!owner->active)
			return false;
		if (inclusive) {
			for (int i = 0; i < D; i++) {
				if (point[i] > max_point[i])
					return false;
				if (point[i] < min_point[i])
					return false;
			}
		} else {
			for (int i = 0; i < D; i++) {
				if (point[i] >= max_point[i])
					return false;
				if (point[i] <= min_point[i])
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
