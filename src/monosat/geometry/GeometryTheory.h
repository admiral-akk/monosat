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

template<unsigned int D, class T>
class GeometryTheorySolver;
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
 	vec<int> shapeToVar;

	struct Trail {
		int level = -1;
		Var prev_var=var_Undef;
		Var next_var=var_Undef;
	};

	vec<Trail> trail;
	Var trailEnd;
	int decisionLevel;

	int theory_index=0;
public:

	GeometryTheorySolver(Solver * S_, int _id = -1) :
	S(S_), id(_id) {
		rnd_seed = opt_random_seed;
		trailEnd = var_Undef;
		decisionLevel = 0;
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
	inline int level(Var v)const {
		return S->level(toSolver(v));
	}
	void undecideTheory(Lit l){
			if (vars[var(l)].isPredicate) {
				detectors[vars[var(l)].index]->undecideTheory(l);
			} else {
				if (sign(l)) 
					under_csg.updateBoolean(false, vars[var(l)].index);
				else 
					over_csg.updateBoolean(true, vars[var(l)].index);
				
				for (auto * d : detectors) {
					d->undecideTheory(l);
				}
			}
			assigns[var(l)] = l_Undef;
	}

	void backtrackAssign(Var v){
			if (vars[v].isPredicate) {
				detectors[vars[v].index]->undecideTheory(mkLit(v,assigns[v]==l_True));
			} else {
				for (auto * d : detectors) {
					d->undecideTheory(mkLit(v,assigns[v] == l_True));
				}
			}
		}

	void backtrackUntil(int untilLevel) {
		while (trailEnd != var_Undef && trail[trailEnd].level > untilLevel) {
			trail[trailEnd].next_var = var_Undef;
			undecideTheory(mkLit(trailEnd,assigns[trailEnd] == l_True));
			assigns[trailEnd] = l_Undef;
			trailEnd = trail[trailEnd].prev_var;
		}
		decisionLevel = untilLevel;
		return;
	}
	inline void newDecisionLevel() {
		decisionLevel++;
	}
	void enqueueTheory(Lit l) {
		assigns[var(l)] = sign(l) ? l_False : l_True;
		if (vars[var(l)].isPredicate) {
			detectors[vars[var(l)].index]->enqueueTheory(l);
		} else {
				if (sign(l)) 
					under_csg.updateBoolean(true, vars[var(l)].index);
				else 
					over_csg.updateBoolean(false, vars[var(l)].index);

			for (auto * d : detectors) {
				d->enqueueTheory(l);
			}
		}
	}
	bool propagateTheory(vec<Lit> & conflict) {
		for (auto * d : detectors) {
			if (!d->propagate(conflict)) {
				return false;
			}
		}
		return true;
	}
	bool solveTheory(vec<Lit> & conflict){
		return propagateTheory(conflict);
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
	void preprocess(){
		for (auto * d : detectors) {
			d->preprocess();
		}
	}

	Var newVar(Var solverVar, int detector, bool isPredicate = false, bool connectToTheory = true) {
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

	void addPoint(std::vector<T> v) {
		assert(D == v.size());
		Point<D,T>* p = new Point<D,T>(v);
		points.emplace_back(p);
	}

	void addPlane(int pointIndex, int vectorIndex) {
		Plane<D,T>* p = new Plane<D,T>(points[pointIndex], points[vectorIndex]);
		planes.emplace_back(p);
	}

	void addPrimative(std::vector<int>* planeIndexVector) {
		std::vector<Plane<D,T>*>* boundary = new std::vector<Plane<D,T>*>();
		for (auto index : *planeIndexVector) {
			boundary->emplace_back(planes[index]);
		}
		PlanePolygon<D,T>* p = new PlanePolygon<D,T>(boundary);
		over_csg.shapes.emplace_back(new Node<D,T>(p));
		under_csg.shapes.emplace_back(new Node<D,T>(p));
	}

	void addConditionalPrimative(std::vector<int>* planeIndexVector, Var outerVar = var_Undef) {
		Var v = newVar(outerVar, over_csg.shapes.size(), false, true);
		std::vector<Plane<D,T>*>* boundary = new std::vector<Plane<D,T>*>();
		for (auto index : *planeIndexVector) {
			boundary->emplace_back(planes[index]);
		}
		PlanePolygon<D,T>* p = new PlanePolygon<D,T>(boundary);
		over_csg.shapes.emplace_back(new Node<D,T>(p,mkLit(outerVar,true)));
		under_csg.shapes.emplace_back(new Node<D,T>(p,mkLit(outerVar,false)));
	}
	
	void addShape(int AIndex, int BIndex, int type) {
		assert(-1 < type && type < 3);
		int n = over_csg.shapes.size();
		over_csg.shapes.push_back(new Node<D,T>(over_csg.getNode(AIndex),over_csg.getNode(BIndex),type));
		over_csg.getNode(AIndex)->parentVector->push_back(over_csg.shapes[n]);
		over_csg.getNode(BIndex)->parentVector->push_back(over_csg.shapes[n]);
		under_csg.shapes.push_back(new Node<D,T>(under_csg.getNode(AIndex),under_csg.getNode(BIndex),type));
		under_csg.getNode(AIndex)->parentVector->push_back(under_csg.shapes[n]);
		under_csg.getNode(BIndex)->parentVector->push_back(under_csg.shapes[n]);
	}
	
	void addConditionalShape(int AIndex, int BIndex, int type, Var outerVar = var_Undef) {
		assert(-1 < type && type < 3);
		int n = over_csg.shapes.size();
		Var v = newVar(outerVar, n, false, true);
		over_csg.shapes.push_back(new Node<D,T>(over_csg.getNode(AIndex),over_csg.getNode(BIndex),type,mkLit(outerVar,true)));
		over_csg.getNode(AIndex)->parentVector->push_back(over_csg.shapes[n]);
		over_csg.getNode(BIndex)->parentVector->push_back(over_csg.shapes[n]);
		under_csg.shapes.push_back(new Node<D,T>(under_csg.getNode(AIndex),under_csg.getNode(BIndex),type,mkLit(outerVar,false)));
		under_csg.getNode(AIndex)->parentVector->push_back(under_csg.shapes[n]);
		under_csg.getNode(BIndex)->parentVector->push_back(under_csg.shapes[n]);
	}
	
	void addShapeContainsPoint(int shape, int point, Var outerVar = var_Undef) {
		Var v = newVar(outerVar, detectors.size(), true, true);
		PointContainmentDetector<D,T>* det = new PointContainmentDetector<D,T>(under_csg.getNode(shape), over_csg.getNode(shape), &under_csg, &over_csg, points[point], lit_Undef);
		detectors.emplace_back(det);
	}

	};
}
#endif /* DGRAPH_H_ */