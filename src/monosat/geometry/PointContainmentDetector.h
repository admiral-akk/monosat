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
#include <map>
using namespace Monosat;

template<unsigned int D, class T>
class PointContainmentDetector : public GeometryDetector {

private: 

	Node<D,T>* under;
	Node<D,T>* over;
	
	CSG<D,T>* under_csg;
	CSG<D,T>* over_csg;
	std::map<Node<D,T>*,bool> under_cache;
	std::map<Node<D,T>*,bool> over_cache;
	Point<D, T>* p;
	Lit l;
 	
	//
	// Point Containment
	//

	//
	// Initialization
	//

	void initialize(Node<D,T>* root, std::map<Node<D,T>*,bool>& cache) {
		switch (root->type) {
			case Primative:
				cache[root] = root->p->contains(p);
				break;
			case Union:
				if (!cache.count(root->left))
					initialize(root->left, cache);
				if (!cache.count(root->right))
					initialize(root->right, cache);
				cache[root] = cache[root->left] || cache[root->right];
				break;
			case Intersection:
				if (!cache.count(root->left))
					initialize(root->left, cache);
				if (!cache.count(root->right))
					initialize(root->right, cache);
				cache[root] = cache[root->left] && cache[root->right];
				break;
			case Difference:
				if (!cache.count(root->left))
					initialize(root->left, cache);
				if (!cache.count(root->right))
					initialize(root->right, cache);
				cache[root] = cache[root->left] && !cache[root->right];
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

	void update(Node<D,T>* root, std::map<Node<D,T>*,bool>& cache) {
		if (!cache.count(root)) 
			return;
		bool old = cache[root];
		initialize(root, cache);
		if (old == cache[root])
			return;
		for (auto node : *(root->parentVector))
			update(node, cache); 
	}

	void update(Var v) {
		update(under_csg->getNode(v), under_cache);
		update(over_csg->getNode(v), over_cache);
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
	void learnPositiveClause(Node<D,T>* root, vec<Lit>& conflict, std::map<Node<D,T>*,bool>& cache) {
		if (cache[root] && root->l != lit_Undef && !sign(root->l)) {
			// Simplest clause is to turn this node on.
			conflict.push(root->l);
			return;
		}
		vec<Lit> temp_A;
		vec<Lit> temp_B;
		switch (root->type) {
			case Union:
				learnPositiveClause(root->left,temp_A,cache);
				learnPositiveClause(root->right,temp_B,cache);
				appendSmallerVector(conflict, temp_A, temp_B);
				break;
			case Intersection:
				learnPositiveClause(root->left,temp_A,cache);
				learnPositiveClause(root->right,temp_B,cache);
				appendVector(conflict, temp_A);
				appendVector(conflict, temp_B);
				break;
			case Difference:
				learnPositiveClause(root->left,temp_A,cache);
				learnNegativeClause(root->right,temp_B,cache);
				appendVector(conflict, temp_A);
				appendVector(conflict, temp_B);
				break;
			default:
				break;
		}
	}

	// Learn clause that must be true for the point to be excluded
	void learnNegativeClause(Node<D,T>* root, vec<Lit>& conflict, std::map<Node<D,T>*,bool>& cache) {
		if (cache[root] && root->l != lit_Undef && sign(root->l)) {
			// Simplest clause is to shut this node off.
			conflict.push(~root->l);
			return;
		}
		vec<Lit> temp_A;
		vec<Lit> temp_B;
		switch (root->type) {
			case Union:
				learnNegativeClause(root->left,temp_A,cache);
				learnNegativeClause(root->right,temp_B,cache);
				appendVector(conflict, temp_A);
				appendVector(conflict, temp_B);
				break;
			case Intersection:
				learnNegativeClause(root->left,temp_A,cache);
				learnNegativeClause(root->right,temp_B,cache);
				appendSmallerVector(conflict, temp_A, temp_B);
				break;
			case Difference:
				learnNegativeClause(root->left,temp_A,cache);
				learnPositiveClause(root->right,temp_B,cache);
				appendSmallerVector(conflict, temp_A, temp_B);
				break;
			default:
				break;
		}
	}

	

public:	

	~PointContainmentDetector() {

	}

	PointContainmentDetector(Node<D,T>* underRoot, Node<D,T>* overRoot, CSG<D,T>* undercsg, CSG<D,T>* overcsg, Point<D,T>* point, Lit literal) {
		this->under = underRoot;
		this->over = overRoot;
		this->under_csg = undercsg;
		this->over_csg = overcsg;
		this->p = point;
		this->l = literal;
	}

	bool propagate(vec<Lit> & conflict) {
		if (under_cache[under] && !sign(l)) {
			// Build a clause that explains why the point is contained in the under approximation
			learnPositiveClause(under, conflict, under_cache);
			return false;
		} else if (!over_cache[over] && sign(l)) {
			// Build a clause that explains why the point isn't contained in the over approximation
			learnNegativeClause(over, conflict, over_cache);
			return false;
		}
	return true;
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

#endif
