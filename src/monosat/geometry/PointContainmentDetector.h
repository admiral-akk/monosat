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
#include "CSGTypes.h"
#include "GeometryTheory.h"
#include <map>

using namespace Monosat;


namespace Monosat {
template<unsigned int D, class T>
class GeometryTheorySolver;

template<unsigned int D, class T>
class PointContainmentDetector : public GeometryDetector {

private: 

	int shape;
	Point<D, T>* p;
	Lit l;

	std::map<int, bool> under_cache;
	std::map<int, bool> over_cache;

	GeometryTheorySolver<D,T>* theory;
	//
	// Point Containment
	//

	//
	// Initialization
	//

	void initialize(int rootIndex, CSG<D,T>* csg, std::map<int,bool>& cache) {
		Node<D,T>* root = csg->getNode(rootIndex);
		switch (root->type) {
		case Primative:
			cache[rootIndex] = root->p->contains(p) && root->active;
			break;
		case Union:
			if (!cache.count(root->left))
				initialize(root->left, csg, cache);
			if (!cache.count(root->right))
				initialize(root->right, csg, cache);
			cache[rootIndex] = getCacheValue(root->left,csg,cache) || getCacheValue(root->right,csg,cache);
			break;
		case Intersection:
			if (!cache.count(root->left))
				initialize(root->left, csg, cache);
			if (!cache.count(root->right))
				initialize(root->right, csg, cache);
			cache[rootIndex] = getCacheValue(root->left,csg,cache) && getCacheValue(root->right,csg,cache);
			break;
		case Difference:
			if (!cache.count(root->left))
				initialize(root->left, csg, cache);
			if (!cache.count(root->right))
				initialize(root->right, csg, cache);
			cache[rootIndex] = getCacheValue(root->left,csg,cache) && !getCacheValue(root->right,csg,cache);
			break;
		default:
			break;
		}
	}

	void initialize() {
		initialize(shape, &(theory->under_csg), under_cache);
		initialize(shape, &(theory->over_csg), over_cache);
	}

	//
	// Updates
	//

	void update(int rootIndex, CSG<D,T>* csg, std::map<int,bool>& cache) {
		Node<D,T>* root = csg->getNode(rootIndex);
		if (!cache.count(rootIndex))
			return;
		bool old = cache[rootIndex];
		initialize(rootIndex, csg, cache);
		if (old == cache[rootIndex])
			return;
		for (auto nodeIndex : *(root->parentVector))
			update(nodeIndex, csg, cache);
	}

	void update(Lit local_l) {
		int localShape = theory->getShape(local_l);

		update(localShape, &(theory->under_csg), under_cache);
		update(localShape, &(theory->over_csg), over_cache);
	}

	//
	// Clause Learning
	//

	void appendVector(vec<Lit>& target, vec<Lit>& toAppend) {
		while (toAppend.size()) {
			target.push(toAppend.last());
			toAppend.pop();
		}
	}

	void appendSmallerVector(vec<Lit>& target, vec<Lit>& toAppend1, vec<Lit>& toAppend2) {
		if (toAppend1.size() < toAppend2.size()) {
			while (toAppend1.size()) {
				target.push(toAppend1.last());
				toAppend1.pop();
			}
		} else {
			while (toAppend2.size()) {
				target.push(toAppend2.last());
				toAppend2.pop();
			}
		}
	}


	// Learn clause that must be true for the point to be contained
	void learnPositiveClause(int rootIndex, CSG<D,T>* csg, vec<Lit>& conflict, std::map<int,bool>& cache) {
		Node<D,T>* root = csg->getNode(rootIndex);
		Var v = theory->shapeToVar[rootIndex];
		if (v != var_Undef && (theory->assigns[v] != l_Undef)) {
			Lit rootLit = mkLit(v,theory->assigns[v] == l_True);
			if (cache[rootIndex] && sign(rootLit)) {
				// Simplest clause is to turn this node on.
				conflict.push(~rootLit);
				return;
			}
		}

		vec<Lit> temp_A;
		vec<Lit> temp_B;
		switch (root->type) {
		case Union:
			learnPositiveClause(root->left,csg,temp_A,cache);
			learnPositiveClause(root->right,csg,temp_B,cache);
			appendSmallerVector(conflict, temp_A, temp_B);
			break;
		case Intersection:
			learnPositiveClause(root->left,csg,temp_A,cache);
			learnPositiveClause(root->right,csg,temp_B,cache);
			appendVector(conflict, temp_A);
			appendVector(conflict, temp_B);
			break;
		case Difference:
			learnPositiveClause(root->left,csg,temp_A,cache);
			learnNegativeClause(root->right,csg,temp_B,cache);
			appendVector(conflict, temp_A);
			appendVector(conflict, temp_B);
			break;
		default:
			break;
		}
	}

	// Learn clause that must be true for the point to be excluded
	void learnNegativeClause(int rootIndex, CSG<D,T>* csg, vec<Lit>& conflict, std::map<int,bool>& cache) {
		Node<D,T>* root = csg->getNode(rootIndex);
		Var v = theory->shapeToVar[rootIndex];
		if (v != var_Undef && (theory->assigns[v] != l_Undef)) {
			Lit rootLit = mkLit(v,theory->assigns[v] == l_True);
			if (cache[rootIndex] && !sign(rootLit)) {
				// Simplest clause is to shut this node off.
				conflict.push(~rootLit);
				return;
			}
		}
		vec<Lit> temp_A;
		vec<Lit> temp_B;
		switch (root->type) {
		case Union:
			learnNegativeClause(root->left,csg,temp_A,cache);
			learnNegativeClause(root->right,csg,temp_B,cache);
			appendVector(conflict, temp_A);
			appendVector(conflict, temp_B);
			break;
		case Intersection:
			learnNegativeClause(root->left,csg,temp_A,cache);
			learnNegativeClause(root->right,csg,temp_B,cache);
			appendSmallerVector(conflict, temp_A, temp_B);
			break;
		case Difference:
			learnNegativeClause(root->left,csg,temp_A,cache);
			learnPositiveClause(root->right,csg,temp_B,cache);
			appendSmallerVector(conflict, temp_A, temp_B);
			break;
		default:
			break;
		}
	}

	bool getCacheValue(int rootIndex, CSG<D,T>* csg, std::map<int,bool>& cache) {
		Node<D,T>* root = csg->getNode(rootIndex);
		return cache[rootIndex] && root->active;
	}



public:	

	~PointContainmentDetector() {

	}

	PointContainmentDetector(GeometryTheorySolver<D,T>* theory, int shape, Point<D,T>* point, Lit literal) {
		this->shape = shape;
		this->theory = theory;
		this->p = point;
		this->l = literal;
	}

	bool propagate(vec<Lit> & conflict) {
		if (l == lit_Undef)
			return true;
		if (getCacheValue(shape, &(theory->under_csg), under_cache) && sign(l)) {
			// Build a clause that explains why the point is contained in the under approximation
			learnNegativeClause(shape, &(theory->under_csg), conflict, under_cache);
			conflict.push(~l);
			return false;
		} else if (!getCacheValue(shape, &(theory->over_csg), over_cache) && !sign(l)) {
			// Build a clause that forces the point to be contained
			learnPositiveClause(shape, &(theory->over_csg), conflict, over_cache);
			conflict.push(~l);
			return false;
		}
		return true;
	}

	void enqueueTheory(Lit new_l, bool owner) {
		if (!owner)
			update(new_l);
		else
			l = new_l;
	}

	void undecideTheory(Lit new_l, bool owner) {
		if (!owner)
			update(new_l);
		else
			l = lit_Undef;
	}

	void buildReason(Lit p, vec<Lit> & reason, CRef marker){};

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
		initialize();
	}

	Lit decide() {
		return lit_Undef;
	}

	void addVar(Var v) {
	}

	void setOccurs(Lit l, bool occurs) {

	}
};
}
#endif
