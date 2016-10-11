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
#ifndef POINT_CONTAINS_DETECTOR_H_
#define POINT_CONTAINS_DETECTOR_H_
#include "GeometryTypes.h"
#include "monosat/core/SolverTypes.h"
#include "GeometryDetector.h"
#include "monosat/core/Config.h"
#include "CSG.h"
#include <map>
using namespace Monosat;

template<unsigned int D, class T>
class PointContainsDetector: public GeometryDetector {

private: 
	
	Node* under;
	Node* over;
	CSG<D,T>* under_csg;
	CSG<D,T>* over_csg;
	std::map<Node*,bool> under_cache;
	std::map<Node*,bool> over_cache;
	Point<D, T> p;
	Lit l;
 	
	//
	// Point Containment
	//

	//
	// Initialization
	//

	void initialize(Node* root, std::map<Node*,bool>& cache) {
		if (!root->box.contains(p)) {
			cache[root] = false;
			return;
		}
		switch (type) {
			case Primative:
				cache[root] = root.p.contains(p);
				break;
			case Union:
				if (!cache.count(root->left))
					initialize(root->left, cache);
				if (cache[root->left])
					cache[root] = true 
				else {
					if (!cache.count(root->right))
						initialize(root->right, cache);
					cache[root] = cache[root->right];
				}
				break;
			case Intersect:
				if (!cache.count(root->left))
					initialize(root->left, cache);
				if (!cache[root->left])
					cache[root] = false 
				else {
					if (!cache.count(root->right))
						initialize(root->right, cache);
					cache[root] = cache[root->right];
				}
				break;
			case Difference:
				if (!cache.count(root->left))
					initialize(root->left, cache);
				if (!cache[root->left])
					cache[root] = false 
				else {
					if (!cache.count(root->right))
						initialize(root->right, cache);
					cache[root] = !cache[root->right];
				}
				break;
			default:
				break;
		}
	}

	void initialize() {
		initialize(under, under_cache);
		initialize(over, over_cache);
	}

	//
	// Updates
	//

	void update(Node* root, std::map<Node*,bool>& cache, Node* head) {
		if (!root) 
			return;
		bool old = cache[root];
		initialize(root, cache);
		if (root == head || old == cache[root])
			return;
		update(root->parent, cache, head); 
	}

	void update(Var v) {
		update(under_csg->boolSwitchMap[v], under_cache, under);
		update(over_csg->boolSwitchMap[v], over_cache, over);
	}

	//
	// Clause Learning
	//

	// Returns learned clause in conflict
	void learnClause(Node* root, bool containsValue, vec<Lit> & conflict, std::map<Node*,bool>& cache) {
		if (containsValue) {

			// We need a clause that will stop the root from containing the point

			if (cache[root] && root->conditional != lit_Undef && sign(root->conditional)) {
				// Simplest clause is to shut this node off.
				conflict.push(~root->conditional);
				return;
			}

			switch (root->type) {

				case Union:
					// Take the shorter of the two clauses the children produce
					vec<Lit> temp_A = new vec<Lit>();
					vec<Lit> temp_B = new vec<Lit>();
					learnClause(root->left,containsValue,temp_A);
					learnClause(root->right,containsValue,temp_B);
					if (temp_A.size() < temp_B.size()) {
						while (temp_A.size()) {
							conflict.push(temp_A.pop());
						}
					} else {
						while (temp_B.size()) {
							conflict.push(temp_B.pop());
						}
					}
					delete temp_A;
					delete temp_B;
					return;

				case Intersection:
					// If either child doesn't contain the point, neither will this node
					learnClause(root->left,containsValue,conflict))
					learnClause(root->right,containsValue,conflict)
					return;

				case Difference:

					// If either the left doesn't contain it, or the right does, then this node won't contain it
					learnClause(root->left,containsValue,conflict))
					learnClause(root->right,!containsValue,conflict)
					return;

				case Primative:
					// At this point, the primative cannot contain the point, so there's nothing we can return to learn.
					return;
				default:
			}
		} else {

			// We need a clause that will force the root to contain the point

			if (cache[root] && root->conditional != lit_Undef && !sign(root->conditional)) {
				// Simplest clause is to turn this node on.
				conflict.push(root->conditional);
				return;
			}

			switch (root->type) {

				case Union:
					// If either child contains the point, then so will this node
					learnClause(root->left,containsValue,conflict))
					learnClause(root->right,containsValue,conflict)
					return;

				case Intersection:
					// Take the shorter of the two clauses the children produce
					vec<Lit> temp_A = new vec<Lit>();
					vec<Lit> temp_B = new vec<Lit>();
					learnClause(root->left,containsValue,temp_A);
					learnClause(root->right,containsValue,temp_B);
					if (temp_A.size() < temp_B.size()) {
						while (temp_A.size()) {
							conflict.push(temp_A.pop());
						}
					} else {
						while (temp_B.size()) {
							conflict.push(temp_B.pop());
						}
					}
					delete temp_A;
					delete temp_B;
					return;

				case Difference:
					// Take the shorter of the two clauses the children produce
					vec<Lit> temp_A = new vec<Lit>();
					vec<Lit> temp_B = new vec<Lit>();
					learnClause(root->left,containsValue,temp_A);
					learnClause(root->right,!containsValue,temp_B);
					if (temp_A.size() < temp_B.size()) {
						while (temp_A.size()) {
							conflict.push(temp_A.pop());
						}
					} else {
						while (temp_B.size()) {
							conflict.push(temp_B.pop());
						}
					}
					delete temp_A;
					delete temp_B;
					return;
					
				case Primative:
					// At this point, the primative cannot contain the point, so there's nothing we can return to learn.
					return;
				default:
			}
		}
	}

public:	

	bool PointContainsDetector<D, T>::propagate(vec<Lit> & conflict) {
		if (under_cache[under] && !sign(l)) {
			// Build a clause that prevents the root of the under approx from containing the point
			learnClause(under, true, conflict, under_cache);
			return false;
		} else if (!over_cache[over] && sign(l)) {
			// Build a clause that forces the root of the over approx to contain the point
			learnClause(over, false, conflict, over_cache);
			return false;
		}
	return true;
	}
	
	void buildReason(Lit p, vec<Lit> & reason, CRef marker)=0;
	
	bool checkSatisfied() {
		return true;
	}
	
	void backtrackUntil(int level) {
		
	}
	
	void printStats() {
		
	}
	
	void printSolution() {
		
	}
	
	void backtrackUntil(Lit p) {
		
	}
	
	void preprocess() {
		return;
	}
	
	Lit decide() {
		return lit_Undef;
	}
	
	void addVar(Var v) {
		unassigned_negatives++;
		unassigned_positives++;
	}
	
	void setOccurs(Lit l, bool occurs) {
		if (!occurs) {
			if (sign(l))
				unassigned_negatives--;
			else
				unassigned_positives--;
		} else {
			if (sign(l))
				unassigned_negatives++;
			else
				unassigned_positives++;
		}
		assert(unassigned_positives >= 0);
		assert(unassigned_negatives >= 0);
	}
	
	void assign(Lit l) {
		Var v = var(l);
		if (sign(l)) {
			unassigned_negatives--;
		} else {
			unassigned_positives--;
		}
		update(v);
		assert(unassigned_positives >= 0);
		assert(unassigned_negatives >= 0);
	}

	void unassign(Lit l) {
		Var v = var(l);
		if (sign(l)) {
			unassigned_negatives++;
		} else {
			unassigned_positives++;
		}
		update(v);
	}
};

#endif
