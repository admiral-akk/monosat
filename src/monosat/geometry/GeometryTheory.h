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

#ifndef GEOMETRY_THEORY_H_
#define GEOMETRY_THEORY_H_

#include "monosat/utils/System.h"
#include "monosat/core/Theory.h"
#include "monosat/core/Config.h"
#include "monosat/mtl/Map.h"
#include "GeometryTypes.h"
#include "Polygon.h"
#include "monosat/utils/System.h"
#include "monosat/core/Solver.h"
#include "GeometryDetector.h"
#include "PointContainmentDetector.h"
#include "CSG.h"
#include <gmpxx.h>

#ifndef NDEBUG
#include <cstdio>
#endif

using namespace Monosat;

namespace Monosat {

template<unsigned int D = 2, class T = int>
 class GeometryTheorySolver: public Theory {

 public:

 	double rnd_seed;

 	int local_q=0;
 private:
 	Solver * S;
 public:
 	int id;


 	CSG<D, T> under_csg;
 	CSG<D, T> over_csg;

 	vec<bool> polarity;

 	std::vector<Point<D,T>*> points;
 	std::vector<Plane<D,T>*> planes;

 	std::vector<GeometryDetector*> detectors;

	struct VarData {
		bool isPredicate :1;
		int index :27;	//the detector this variable belongs to, or its edge number, if it is an edge variable
		Var solverVar;
	};

 	vec<lbool> assigns;
 	vec<VarData> vars;
 	vec<Var> shapeToVar;

	/*struct Trail {
		int level = -1;
		Var prev_var=var_Undef;
		Var next_var=var_Undef;
	};

	vec<Trail> trail;
	Var trailEnd;
	int decisionLevel;*/

	int theory_index=0;

public:

	GeometryTheorySolver(Solver * S_, int _id = -1) :
	S(S_), id(_id) {
		rnd_seed = opt_random_seed;
		//trailEnd = var_Undef;
		//decisionLevel = 0;
	}
	
	~GeometryTheorySolver() {
		for (auto * d : detectors) {
			delete (d);
		}
		for (auto * p : points) {
			delete (p);
		}
		for (auto * p : planes) {
			delete (p);
		}
	}

    int getPriority()const{
		return priority;
	}
    void setPriority(int p){
    	priority=p;
    }

    double & getActivity(){
		return activity;
	}
    void setActivity(double p){
    	activity=p;
    }

	inline int getTheoryIndex() {
		return theory_index;
	}
	inline void setTheoryIndex(int id) {
		theory_index = id;
	}

	inline Var toSolver(Var v) const {
		assert(v < vars.size());
		return vars[v].solverVar;
	}

	inline Lit toSolver(Lit l) const {
		return mkLit(vars[var(l)].solverVar, sign(l));
	}

	void toSolver(vec<Lit> & c) {
		for (int i = 0; i < c.size(); i++) {
			c[i] = toSolver(c[i]);
		}
	}

	vec<Lit> tmp_clause;

	void makeEqual(Lit l1, Lit l2) {
		Lit o1 = toSolver(l1);
		Lit o2 = toSolver(l2);
		tmp_clause.clear();
		tmp_clause.push(~o1);
		tmp_clause.push(o2);
		S->addClauseSafely(tmp_clause);
		tmp_clause.clear();
		tmp_clause.push(o1);
		tmp_clause.push(~o2);
		S->addClauseSafely(tmp_clause);
	}
	void makeEqualInSolver(Lit o1, Lit o2) {
		tmp_clause.clear();
		tmp_clause.push(~o1);
		tmp_clause.push(o2);
		S->addClauseSafely(tmp_clause);
		tmp_clause.clear();
		tmp_clause.push(o1);
		tmp_clause.push(~o2);
		S->addClauseSafely(tmp_clause);
	}
	inline int level(Var v)const {
		return S->level(toSolver(v));
	}
	void undecideTheory(Lit l){
			std::cout << "Dequeue: " << toInt(l) << std::endl;
			if (vars[var(l)].isPredicate) {
				detectors[vars[var(l)].index]->undecideTheory(l, true);
			} else {
				under_csg.updateBoolean(!polarity[vars[var(l)].index], vars[var(l)].index);
				over_csg.updateBoolean(polarity[vars[var(l)].index], vars[var(l)].index);
				
				for (auto * d : detectors) {
					d->undecideTheory(l, false);
				}
			}
			assigns[var(l)] = l_Undef;
	}

	void backtrackAssign(Var v){
			/*if (vars[v].isPredicate) {
				detectors[vars[v].index]->undecideTheory(mkLit(v,assigns[v]==l_True), true);
			} else {
				for (auto * d : detectors) {
					d->undecideTheory(mkLit(v,assigns[v] == l_True), false);
				}
			}*/
		}

	void backtrackUntil(int untilLevel) {
		/*while (trailEnd != var_Undef && trail[trailEnd].level > untilLevel) {
			trail[trailEnd].next_var = var_Undef;
			undecideTheory(mkLit(trailEnd,assigns[trailEnd] == l_True));
			assigns[trailEnd] = l_Undef;
			trailEnd = trail[trailEnd].prev_var;
		}
		decisionLevel = untilLevel;*/
		return;
	}
	inline void newDecisionLevel() {
		//decisionLevel++;
	}
	void enqueueTheory(Lit l) {
		std::cout << "Enqueue: " << toInt(l) << std::endl;
		assigns[var(l)] = !sign(l) ? l_True : l_False;
		if (vars[var(l)].isPredicate) {
			detectors[vars[var(l)].index]->enqueueTheory(l, true);
		} else {
				if (sign(l)) {
					if (polarity[vars[var(l)].index]) {
						over_csg.updateBoolean(false, vars[var(l)].index);
					} else {
						under_csg.updateBoolean(false, vars[var(l)].index);
					}
				} else {
					if (polarity[vars[var(l)].index]) {
						under_csg.updateBoolean(true, vars[var(l)].index);
					} else {
						over_csg.updateBoolean(true, vars[var(l)].index);
					}
				}
			for (auto * d : detectors) {
				d->enqueueTheory(l, false);
			}
		}
	}
	bool propagateTheory(vec<Lit> & conflict) {
		for (auto * d : detectors) {
			if (!d->propagate(conflict)) {
				std::cout << "Conflict:  ";
				for (int i = 0; i < conflict.size(); i++) {
					std::cout << toInt(conflict[i]) << " ";
				}
				std::cout << std::endl;
				toSolver(conflict);
				return false;
			}
		}

		return true;
	}
	bool solveTheory(vec<Lit> & conflict){
		bool ret = propagateTheory(conflict);
		assert(ret);
		return ret;
	}
	Lit decideTheory() {
		return lit_Undef;
	}
	bool supportsDecisions() {
		return false;
	}

	//Lazily construct the reason clause explaining this propagation
	void buildReason(Lit p, vec<Lit> & reason, CRef reason_marker){
		return buildReason(p,reason);
	}
	bool supportsLazyBacktracking(){
		return false;
	}



protected:
	void buildReason(Lit p, vec<Lit> & reason){

	}
public:
	//Informs the theory solver about whether this literal (with this polarity!) ever occurs in its parent solver
	void setLiteralOccurs(Lit l, bool occurs) {
		
	}
	void printStats(int detailLevel = 0) {
		
	}
	bool check_propagated(){
		return true;
	}
	bool check_solved() {
		return true;
	}
	void printSolution() {
		
	}
	void writeTheoryWitness(std::ostream& write_to) {
		//do nothing
	}

	bool getPolarity(int index) {
		std::cout << over_csg.getNode(index)->parentVector.size() << std::endl;
		if (!over_csg.getNode(index)->parentVector.size()) {
			return true;
		}
		int parentIndex = over_csg.getNode(index)->parentVector[0];
		return !(over_csg.getNode(parentIndex)->type == Difference && over_csg.getNode(parentIndex)->right == index) ? polarity[parentIndex] : !polarity[parentIndex];
	}

	void initPolarity() {
		for (int i = polarity.size()-1; i > -1; i--) {
			polarity[i] = getPolarity(i);
			if (shapeToVar[i] != var_Undef) {
				over_csg.shapes[i].active = polarity[i];
				under_csg.shapes[i].active = !polarity[i];
			}
		}
	}

	void preprocess() {
		initPolarity();
		for (auto * d : detectors) {
			d->preprocess();
		}
	}

	Var newVar(Var solverVar, int detector, bool isPredicate = false, bool connectToTheory = true) {
		std::cout << "Solver var: " << solverVar << std::endl;
		while (S->nVars() <= solverVar)
			S->newVar();

		if(S->hasTheory(solverVar) &&  S->getTheoryID(solverVar)==getTheoryIndex()){
			return S->getTheoryVar(solverVar);
		}

		Var v = vars.size();
		vars.push();
		vars[v].isPredicate = isPredicate;
		vars[v].index = detector;
		vars[v].solverVar = solverVar;


		assigns.push(l_Undef);

		if (connectToTheory) {
			S->setTheoryVar(solverVar, getTheoryIndex(), v);
			assert(toSolver(v) == solverVar);
		}
		return v;
	}

	int addPoint(vec<T>* v) {
		assert(D == v->size());
		Point<D,T>* p = new Point<D,T>(*v);
		points.emplace_back(p);
		return points.size() - 1;
	}

	int addPlane(int pointIndex, int vectorIndex) {
		Plane<D,T>* p = new Plane<D,T>(points[pointIndex], points[vectorIndex]);
		planes.emplace_back(p);
		return planes.size() - 1;
	}

	int addPrimative(vec<int>* planeIndexVector) {
		std::vector<Plane<D,T>*>* boundary = new std::vector<Plane<D,T>*>();
		for (auto index : *planeIndexVector) {
			boundary->emplace_back(planes[index]);
		}
		PlanePolygon<D,T>* p = new PlanePolygon<D,T>(boundary);
		int n = over_csg.shapes.size();
		over_csg.addPrimative(p,true,n);
		under_csg.addPrimative(p,true,n);

		polarity.growTo(n+1);

		shapeToVar.growTo(n+1);
		shapeToVar[n] = var_Undef;

		return n;
	}

	int addConditionalPrimative(vec<int>* planeIndexVector, Var outerVar = var_Undef) {
		std::cout << "Solver var: " << outerVar << std::endl;
		Var v = newVar(outerVar, over_csg.shapes.size(), false, true);
		std::vector<Plane<D,T>*>* boundary = new std::vector<Plane<D,T>*>();
		for (auto index : *planeIndexVector) {
			boundary->emplace_back(planes[index]);
		}
		PlanePolygon<D,T>* p = new PlanePolygon<D,T>(boundary);
		int n = over_csg.shapes.size();
		over_csg.addPrimative(p,true,n);
		under_csg.addPrimative(p,false,n);

		polarity.growTo(n+1);

		shapeToVar.growTo(n+1);
		shapeToVar[n] = v;

		return n;
	}
	
	int addShape(int AIndex, int BIndex, int type) {
		assert(-1 < type && type < 3);
		int n = over_csg.shapes.size();
		over_csg.addShape(AIndex,BIndex,type,true, n);
		under_csg.addShape(AIndex,BIndex,type, true,n);

		polarity.growTo(n+1);

		shapeToVar.growTo(n+1);
		shapeToVar[n] = var_Undef;

		return n;
	}
	
	int addConditionalShape(int AIndex, int BIndex, int type, Var outerVar = var_Undef) {
		assert(-1 < type && type < 3);
		int n = over_csg.shapes.size();
		Var v = newVar(outerVar, n, false, true);
		over_csg.addShape(AIndex, BIndex, type, true, n);
		under_csg.addShape(AIndex, BIndex, type, true, n);

		polarity.growTo(n+1);

		shapeToVar.growTo(n+1);
		shapeToVar[n] = v;

		return n;
	}
	
	void addShapeContainsPoint(int shape, int point, Var outerVar = var_Undef) {
		Var v = newVar(outerVar, detectors.size(), true, true);
		PointContainmentDetector<D,T>* det = new PointContainmentDetector<D,T>(this, shape, points[point], lit_Undef);
		detectors.emplace_back(det);
	}

	int getShape(Lit l) {
		return vars[var(l)].index;
	}

	};
}
#endif /* DGRAPH_H_ */
