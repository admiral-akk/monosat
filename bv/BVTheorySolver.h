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

#ifndef COMPARISON_THEORY_H_
#define COMPARISON_THEORY_H_

#include "utils/System.h"
#include "core/Theory.h"

#include "core/SolverTypes.h"
#include "mtl/Map.h"

#include "BVTheory.h"
#include "utils/System.h"
#include "core/TheorySolver.h"

#include <vector>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <iostream>
template<typename Weight>
class BVTheorySolver;

namespace Monosat {



template<typename Weight>
class BVTheorySolver: public Theory {
public:


	struct Assignment {
		bool isComparator :1;
		bool assign :1;
		int bvID:30;
		Var var;
		Assignment(bool isComparator, bool assign, int bvID, Var v):isComparator(isComparator),assign(assign),bvID(bvID),var(v){

		}
	};

	struct Comparison{
		Weight w;
		Lit l;

		int bvID:30;
		int is_lt:1;
		int is_strict:1;
	};


	class BitVector{
		BVTheorySolver * outer;
		int id;
	public:
		BitVector():outer(nullptr),id(0){

			}
		BitVector(BVTheorySolver & outer, int id):outer(&outer),id(id){

		}
		BitVector(const BitVector & copy):outer(copy.outer),id(copy.id){

			}
		BitVector(const BitVector && move):outer(move.outer),id(move.id){

			}
		BitVector& operator=(BitVector &  other) {
			outer= other.outer;
			id=other.id;
			return *this;
		}

		int getID(){
			return id;
		}
		Weight & getUnder(){
			return outer->getUnderApprox(id);
		}
		Weight & getOver(){
			return outer->getOverApprox(id);
		}

		vec<Lit> & getBits(){
			return outer->getBits(id);
		}
	};

	//typedef void (*CallBack)(int) ;

	double rnd_seed;

private:
	TheorySolver * S;

public:
	int id;

	vec<lbool> assigns;
	CRef comparisonprop_marker;
	CRef bvprop_marker;

	vec<Assignment> trail;
	vec<int> trail_lim;

	//Bitvectors are unsigned, and have least significant bit at index 0
	vec<vec<Lit> > bitvectors;
	vec<Comparison> comparisons;
	vec<vec<int> > comparisons_lt; //for each bitvector, comparisons are all to unique values, and in ascending order of compareTo.
	vec<vec<int> > comparisons_gt; //for each bitvector, comparisons are all to unique values, and in ascending order of compareTo.
	vec<vec<int> > comparisons_leq; //for each bitvector, comparisons are all to unique values, and in ascending order of compareTo.
	vec<vec<int> > comparisons_geq; //for each bitvector, comparisons are all to unique values, and in ascending order of compareTo.

	vec<Weight> under_approx;
	vec<Weight> over_approx;
	vec<int> theoryIds;
	vec<int> altered_bvs;
	vec<bool> alteredBV;

	vec<bool> in_backtrack_queue;
	vec<int> backtrack_queue;
	vec<BVTheory*> theories;
	vec<BVTheory*> actual_theories;
public:


	//vec<int> marker_map;

	bool requiresPropagation = true;

	vec<char> seen;
	vec<int> to_visit;
	vec<Lit> tmp_clause;
	//Data about local theory variables, and how they connect to the sat solver's variables
	struct VarData {

		int occursPositive :1;
		int occursNegative :1;

		int bvID:30;
		int comparisonID;
		Var solverVar;
	};

	vec<VarData> vars;
	int theory_index = 0;
public:

	double reachtime = 0;
	double unreachtime = 0;
	double pathtime = 0;
	double propagationtime = 0;
	long stats_propagations = 0;
	long stats_num_conflicts = 0;
	long stats_decisions = 0;
	long stats_num_reasons = 0;

	double reachupdatetime = 0;
	double unreachupdatetime = 0;
	double stats_initial_propagation_time = 0;
	double stats_decision_time = 0;
	double stats_reason_initial_time = 0;
	double stats_reason_time = 0;
	long num_learnt_paths = 0;
	long learnt_path_clause_length = 0;
	long num_learnt_cuts = 0;
	long learnt_cut_clause_length = 0;
	long stats_pure_skipped = 0;
	long stats_mc_calls = 0;
	long stats_propagations_skipped = 0;


	BVTheorySolver(TheorySolver * S ) :
			S(S){
		rnd_seed = opt_random_seed;
		S->addTheory(this);
		comparisonprop_marker = S->newReasonMarker(this);
		bvprop_marker = S->newReasonMarker(this);

	}
	~BVTheorySolver(){

	}
	TheorySolver * getSolver(){
		return S;
	}

	bool hasTheory(int bvID){
		assert(bvID>=0);
		return theoryIds[bvID]>=0;
	}
	BVTheory * getTheory(int bvID){
		assert(hasTheory(bvID));
		return theories[theoryIds[bvID]];
	}

	void addTheory(BVTheory* theory){
		theories.growTo(theory->getTheoryIndexBV()+1);
		actual_theories.push(theory);
		theories[theory->getTheoryIndexBV()]=theory;
	}

	void printStats(int detailLevel) {

	}
	
	void writeTheoryWitness(std::ostream& write_to) {

	}
	
	inline int getTheoryIndex() {
		return theory_index;
	}
	inline void setTheoryIndex(int id) {
		theory_index = id;
	}


	BitVector getBV(int bvID){
		return BitVector(*this,bvID);
	}

	inline bool isComparisonVar(Var v) const{
		assert(v < vars.size());
		return vars[v].comparisonID>-1;
	}
	inline int getComparisonID(Var v) const {
		assert(isComparisonVar(v));
		return vars[v].comparisonID;
	}
	inline Comparison& getComparison(int comparisonID){
		return comparisons[comparisonID];
	}
	inline int getbvID(Var v) const{

		return vars[v].bvID;
	}

	inline int getDetector(Var v) const {
		assert(!isComparisonVar(v));
		return vars[v].detector_edge;
	}
	


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
	void addClause(Lit l1) {
		Lit o1 = toSolver(l1);
		tmp_clause.clear();
		tmp_clause.push(o1);
		S->addClauseSafely(tmp_clause);
	}
	void addClause(Lit l1, Lit l2) {
		Lit o1 = toSolver(l1);
		Lit o2 = toSolver(l2);
		tmp_clause.clear();
		tmp_clause.push(o1);
		tmp_clause.push(o2);

		S->addClauseSafely(tmp_clause);
	}
	void addClause(Lit l1, Lit l2, Lit l3) {
		Lit o1 = toSolver(l1);
		Lit o2 = toSolver(l2);
		Lit o3 = toSolver(l3);
		tmp_clause.clear();
		tmp_clause.push(o1);
		tmp_clause.push(o2);
		tmp_clause.push(o3);
		S->addClauseSafely(tmp_clause);
	}
	void addClause(vec<Lit> & c) {
		tmp_clause.clear();
		c.copyTo(tmp_clause);
		toSolver(tmp_clause);
		S->addClauseSafely(tmp_clause);
	}
	void addClauseSafely(vec<Lit> & c) {
		tmp_clause.clear();
		c.copyTo(tmp_clause);
		toSolver(tmp_clause);
		
		S->addClauseSafely(tmp_clause);
	}
	

	Var newVar(Var solverVar=var_Undef, int bvID=-1, int comparisonID=-1, bool connectToTheory = true) {
		if(solverVar==var_Undef){
			solverVar = S->newVar();
		}
		Var v = vars.size();
		if (connectToTheory) {
			S->newTheoryVar(solverVar, getTheoryIndex(), v);

		}else{
			while (S->nVars() <= solverVar)
				S->newVar();
		}


		vars.push();
		vars[v].occursPositive=false;
		vars[v].occursNegative=false;
		vars[v].bvID=bvID;
		vars[v].comparisonID=comparisonID;
		vars[v].solverVar = solverVar;

		assigns.push(l_Undef);
		if (connectToTheory) {
			assert(toSolver(v) == solverVar);
		}
		return v;
	}
	inline int level(Var v) {
		return S->level(toSolver(v));
	}
	inline int decisionLevel() {
		return trail_lim.size(); //S->decisionLevel();
	}
	inline int nVars() const {
		return vars.size(); //S->nVars();
	}
	inline Var toSolver(Var v) {
		//return v;
		assert(v < vars.size());
		//assert(S->hasTheory(vars[v].solverVar));
		//assert(S->getTheoryVar(vars[v].solverVar)==v);
		return vars[v].solverVar;
	}
	
	inline Lit toSolver(Lit l) {
		//assert(S->hasTheory(vars[var(l)].solverVar));
		//assert(S->getTheoryVar(vars[var(l)].solverVar)==var(l));
		return mkLit(vars[var(l)].solverVar, sign(l));
	}
	
	void toSolver(vec<Lit> & c) {
		for (int i = 0; i < c.size(); i++) {
			c[i] = toSolver(c[i]);
		}
	}
	
	inline lbool value(Var v) {
		if (assigns[v] != l_Undef)
			assert(S->value(toSolver(v)) == assigns[v]);
		
		return assigns[v]; //S->value(toSolver(v));
	}
	inline lbool value(Lit l) {
		if (assigns[var(l)] != l_Undef) {
			assert(S->value(toSolver(l)) == (assigns[var(l)] ^ sign(l)));
		}
		return assigns[var(l)] ^ sign(l);
	}
	inline lbool dbg_value(Var v) {
		return S->value(toSolver(v));
	}
	inline lbool dbg_value(Lit l) {
		return S->value(toSolver(l));
	}

	void enqueueEager(Lit l, CRef reason_marker){
		enqueue(l,reason_marker);
		//if newly created lits are enqueued, then they must provide a reason eagerly (so that complex propagation dependencies can be handled correctly by the solver, instead of having to be sorted out in the theories).
		/*static int iter = 0;
		++iter;
		vec<Lit>  reason;
		buildReason(l,reason,reason_marker);

		Lit sl = toSolver(l);

		S->addClauseSafely(reason);

		//the lit must have been propagated by this clause (or, alternatively, the solver might now have a conflict).
		if(S->value(sl)==l_True)
			enqueueTheory(l);*/
	}

	inline bool enqueue(Lit l, CRef reason) {
		assert(assigns[var(l)]==l_Undef);
		
		Lit sl = toSolver(l);
		if (S->enqueue(sl, reason)) {
			enqueueTheory(l);
			return true;
		} else {
			return false;
		}
	}

	void backtrackUntil(int level) {
		static int it = 0;
		
		bool changed = false;
		//need to remove and add edges in the two graphs accordingly.
		if (trail_lim.size() > level) {
			
			int stop = trail_lim[level];
			for (int i = trail.size() - 1; i >= trail_lim[level]; i--) {
				
				Assignment & e = trail[i];
				assert(assigns[e.var]!=l_Undef);
				if (e.isComparator) {


					if (e.assign) {
						//g_unders[bvID]->disableTransition(edgeID, input,output);

					} else {
						//g_overs[bvID]->enableTransition(edgeID,input,output);

					}
				} else {
					if(hasTheory(e.bvID)){
						if(!in_backtrack_queue[e.bvID]){
							backtrack_queue.push(e.bvID);
							in_backtrack_queue[e.bvID]=true;
						}
					}
					//status.bvAltered(e.bvID);
				}
				assigns[e.var] = l_Undef;
				//changed = true;
			}
			trail.shrink(trail.size() - stop);
			trail_lim.shrink(trail_lim.size() - level);
			assert(trail_lim.size() == level);
			
			if (changed) {
				//requiresPropagation = true;

			}
			

		}
		while(backtrack_queue.size()){
			int bvID = backtrack_queue.last();
			backtrack_queue.pop();
			in_backtrack_queue[bvID]=false;
			getTheory(bvID)->backtrackBV(bvID);
		}

		
	};

	Lit decideTheory() {

		return lit_Undef;
	}
	
	void backtrackUntil(Lit p) {
		assert(value(p)!=l_False);
		if(value(p)!=l_True)
			return;
		assert(value(p)==l_True);
		int i = trail.size() - 1;
		for (; i >= 0; i--) {
			Assignment e = trail[i];
			if (var(p) == e.var) {
				assert(sign(p) != e.assign);
				break;
			}
			if (e.isComparator) {

				assert(assigns[e.var]!=l_Undef);

				if (e.assign) {
				//	g_unders[bvID]->disableTransition(edgeID,input,output);
				} else {
				//	g_overs[bvID]->enableTransition(edgeID,input,output);
				}
			} else {
				//if(bv_callbacks[e.bvID])
				//	(*bv_callbacks[e.bvID])(e.bvID);
				//status(e.bvID);
				if(hasTheory(e.bvID)){
					if(!in_backtrack_queue[e.bvID]){
						backtrack_queue.push(e.bvID);
						in_backtrack_queue[e.bvID]=true;
					}
				}
				//status.bvAltered(e.bvID);
			}
			assigns[e.var] = l_Undef;
		}
		
		trail.shrink(trail.size() - (i + 1));
		//if(i>0){
		requiresPropagation = true;

		while(backtrack_queue.size()){
			int bvID = backtrack_queue.last();
			backtrack_queue.pop();
			in_backtrack_queue[bvID]=false;
			getTheory(bvID)->backtrackBV(bvID);
		}
	}
	;

	void newDecisionLevel() {
		trail_lim.push(trail.size());
	}
	;

	void buildReason(Lit p, vec<Lit> & reason,CRef marker) {
		static int iter = 0;
		++iter;
		assert(value(p)!=l_False);

		assert(marker != CRef_Undef);
		int pos = CRef_Undef - marker;
		//int d = marker_map[pos];
		//double initial_start = rtime(1);
		double start = rtime(1);

		//if the reason is being constructed eagerly, then p won't be assigned yet, and so wont be on the trail, so we skip this.
		backtrackUntil(p);
		assert(value(p)!=l_False);
		if (marker == comparisonprop_marker) {
			reason.push(p);
			Var v = var(p);
			int bvID = getbvID(v);
			updateApproximations(bvID);
			if(isComparisonVar(v)){
				int comparisonID = getComparisonID(v);
				if(comparisons[comparisonID].is_lt){
					if(comparisons[comparisonID].is_strict){
						if(!sign(p)){
							buildValueLTReason(bvID,comparisonID,reason);
						}else{
							buildValueGEQReason(bvID,comparisonID,reason);
						}
					}else{
						if(!sign(p)){
							buildValueLEQReason(bvID,comparisonID,reason);
						}else{
							buildValueGTReason(bvID,comparisonID,reason);
						}
					}
				}else{
					if(comparisons[comparisonID].is_strict){
						if(!sign(p)){
							buildValueGTReason(bvID,comparisonID,reason);
						}else{
							buildValueLEQReason(bvID,comparisonID,reason);
						}
					}else{
						if(!sign(p)){
							buildValueGEQReason(bvID,comparisonID,reason);
						}else{
							buildValueLTReason(bvID,comparisonID,reason);
						}
					}
				}
			}else{

			}

		} else if (marker == bvprop_marker) {
			reason.push(p);
			Var v = var(p);

		}  else {
			assert(false);
		}

		toSolver(reason);
		double finish = rtime(1);
		stats_reason_time += finish - start;
		stats_num_reasons++;
		//stats_reason_initial_time+=start-initial_start;
		if(reason.size()<2){
			int a=1;
		}
	}


	void preprocess() {

	}
	void setLiteralOccurs(Lit l, bool occurs) {
		/*if (isEdgeVar(var(l))) {
			//don't do anything
		} else {
			//this is a graph property detector var
			if (!sign(l) && vars[var(l)].occursPositive != occurs)
				detectors[getDetector(var(l))]->setOccurs(l, occurs);
			else if (sign(l) && vars[var(l)].occursNegative != occurs)
				detectors[getDetector(var(l))]->setOccurs(l, occurs);
		}*/
		
	}
	
	void enqueueTheory(Lit l) {
		Var v = var(l);
		
		int lev = level(v);
		while (lev > trail_lim.size()) {
			newDecisionLevel();
		}
		
		if (assigns[var(l)] != l_Undef) {
			return;			//this is already enqueued.
		}
		assert(assigns[var(l)]==l_Undef);
		assigns[var(l)] = sign(l) ? l_False : l_True;
		requiresPropagation = true;
		//printf("enqueue %d\n", dimacs(l));
		
#ifndef NDEBUG
		{
			for (int i = 0; i < trail.size(); i++) {
				assert(trail[i].var != v);
			}
		}
#endif
		

		
		if (!isComparisonVar(var(l))) {
			
			int bvID = getbvID(v);
			trail.push( { false, !sign(l),bvID, v });

			if(!alteredBV[bvID]){
				alteredBV[bvID]=true;
				altered_bvs.push(bvID);

			}

		} else {

			int bvID = getbvID(var(l));
			int comparisonID = getComparisonID(var(l)); //v-min_edge_var;
			//status.comparisonAltered(bvID, comparisonID);
			trail.push( { true, !sign(l),bvID, v });
			//trail.push( { true, !sign(l),edgeID, v });
			if(!alteredBV[bvID]){
				alteredBV[bvID]=true;
				altered_bvs.push(bvID);

			}
		}
	}
	;

	void updateApproximations(int bvID){
		static int iter = 0;
		++iter;
		std::cout<< "bv update " << iter << " for " << bvID << ": ";
#ifndef NDEBUG
		for(int i = 0;i<vars.size();i++){
			if(value(i)==l_True){
				std::cout << "1";
			}else if (value(i)==l_False){
				std::cout << "0";
			}else{
				std::cout << "x";
			}
		}
		std::cout<<"\n";
#endif
		vec<Lit> & bv = bitvectors[bvID];
		under_approx[bvID]=0;
		over_approx[bvID]=0;
		for(int i = 0;i<bv.size();i++){
			lbool val = value(bv[i]);
			if(val==l_True){
				Weight bit = 1<<i;
				under_approx[bvID]+=bit;
				over_approx[bvID]+=bit;
			}else if (val==l_False){

			}else{
				Weight bit = 1<<i;
				over_approx[bvID]+=bit;
			}
		}

		for(int lt:comparisons_lt[bvID]){
			Comparison & c = comparisons[lt];
			if(value( c.l)==l_True && over_approx[bvID]>=c.w){
				over_approx[bvID]=c.w-1;
			}else if (value(c.l)==l_False && under_approx[bvID]<c.w){
				under_approx[bvID]=c.w;
			}
		}

		for(int leq:comparisons_leq[bvID]){
			Comparison & c = comparisons[leq];
			if(value( c.l)==l_True && over_approx[bvID]>c.w){
				over_approx[bvID]=c.w;
			}else if (value(c.l)==l_False && under_approx[bvID]<=c.w){
				under_approx[bvID]=c.w+1;
			}
		}

		for(int gt:comparisons_gt[bvID]){
			Comparison & c = comparisons[gt];
			if(value( c.l)==l_True && under_approx[bvID]<=c.w){
				under_approx[bvID]=c.w+1;
			}else if (value(c.l)==l_False && over_approx[bvID]>c.w){
				over_approx[bvID]=c.w;
			}
		}

		for(int geq:comparisons_geq[bvID]){

			Comparison & c = comparisons[geq];
			if(value( c.l)==l_True && under_approx[bvID]<c.w){
				under_approx[bvID]=c.w;
			}else if (value(c.l)==l_False && over_approx[bvID]>=c.w){
				over_approx[bvID]=c.w-1;
			}
		}

	}

	Weight & getUnderApprox(int bvID){


		return under_approx[bvID];
	}
	Weight & getOverApprox(int bvID){


		return over_approx[bvID];
	}

	vec<Lit> & getBits(int bvID){
		return bitvectors[bvID];
	}

	bool checkApproxUpToDate(int bvID){
#ifndef NDEBUG
		vec<Lit> & bv = bitvectors[bvID];
		Weight under =0;
		Weight over=0;
		for(int i = 0;i<bv.size();i++){
			lbool val = value(bv[i]);
			if(val==l_True){
				Weight bit = 1<<i;
				under+=bit;
				over+=bit;
			}else if (val==l_False){

			}else{
				Weight bit = 1<<i;
				over+=bit;
			}
		}

		for(int lt:comparisons_lt[bvID]){
			Comparison & c = comparisons[lt];
			if(value( c.l)==l_True && over>=c.w){
				over=c.w-1;
			}else if (value(c.l)==l_False && under<c.w){
				under=c.w;
			}
		}
		for(int leq:comparisons_leq[bvID]){
			Comparison & c = comparisons[leq];
			if(value( c.l)==l_True && over>c.w){
				over=c.w;
			}else if (value(c.l)==l_False && under<=c.w){
				under=c.w+1;
			}
		}
		for(int gt:comparisons_gt[bvID]){
			Comparison & c = comparisons[gt];
			if(value( c.l)==l_True && under<=c.w){
				under=c.w+1;
			}else if (value(c.l)==l_False && over>c.w){
				over=c.w;
			}
		}
		for(int geq:comparisons_geq[bvID]){
			Comparison & c = comparisons[geq];
			if(value( c.l)==l_True && under<c.w){
				under=c.w;
			}else if (value(c.l)==l_False && over>=c.w){
				over=c.w-1;
			}
		}

		assert(under==under_approx[bvID]);
		assert(over==over_approx[bvID]);
#endif
		return true;
	}

	bool dbg_synced(){
#ifndef NDEBUG
		for(Var v = 0;v<nVars();v++){
			assert(value(v)==dbg_value(v));
		}
#endif
		return true;
	}

	bool propagateTheory(vec<Lit> & conflict) {

		stats_propagations++;
		assert(dbg_synced());
		if (!requiresPropagation) {
			stats_propagations_skipped++;

			return true;
		}
		printf("bv prop %d\n",stats_propagations);
		if(stats_propagations==81){
			int a =1;
		}
		bool any_change = false;
		double startproptime = rtime(1);
		//static vec<int> detectors_to_check;
		
		conflict.clear();

		
		//while(altered_bvs.size()){
			//int bvID = altered_bvs.last();
		for(int bvID = 0;bvID<bitvectors.size();bvID++){
			//assert(alteredBV[bvID]);

			updateApproximations(bvID);

			Weight & underApprox = under_approx[bvID];
			Weight & overApprox = over_approx[bvID];
			std::cout<<"bv: " << bvID << " (" <<underApprox << ", " <<overApprox << " )\n";

			vec<int> & compares_lt = comparisons_lt[bvID];
			//update over approx lits
			for(int i = 0;i<compares_lt.size();i++){
				int comparisonID = compares_lt[i];
				Weight & lt = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (overApprox<lt){
					if(value(l)==l_True){
						//do nothing
						std::cout<<"nothing bv " << bvID << "< " << lt << "\n";
					}else if (value(l)==l_False){
						std::cout<<"conflict bv " << bvID << "< " << lt << "\n";
						assert(value(l)==l_False);
						assert(dbg_value(l)==l_False);
						conflict.push(l);
						buildValueLTReason(bvID,comparisonID,conflict);
						toSolver(conflict);
						return false;
					}else {
						std::cout<<"propagate bv " << bvID << "< " << lt << "\n";
						assert(value(l)==l_Undef);
						enqueue(l, comparisonprop_marker);
					}

				}else{
					//break;
				}
			}

			//update under approx lits
			for(int i = compares_lt.size()-1;i>=0;i--){
				int comparisonID = compares_lt[i];
				Weight & lt = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (underApprox>=lt){
					if(value(l)==l_True){
						std::cout<<"conflict neg bv " << bvID << "< " << lt << "\n";
						conflict.push(~l);
						buildValueGEQReason(bvID,comparisonID,conflict);
						toSolver(conflict);
						return false;
					}else if (value(l)==l_False){
						//do nothing
						std::cout<<"nothing neg bv " << bvID << "< " << lt << "\n";
					}else {
						std::cout<<"propagate neg bv " << bvID << "< " << lt << "\n";
						assert(value(l)==l_Undef);
						enqueue(~l, comparisonprop_marker);
					}

				}else{
					//break;
				}
			}

			vec<int> & compares_leq = comparisons_leq[bvID];
			//update over approx lits
			for(int i = 0;i<compares_leq.size();i++){
				int comparisonID = compares_leq[i];
				Weight & leq = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (overApprox<=leq){
					if(value(l)==l_True){
						std::cout<<"nothing bv " << bvID << "<= " << leq << "\n";
						//do nothing
					}else if (value(l)==l_False){
						std::cout<<"conflict bv " << bvID << "<= " << leq << "\n";
						assert(value(l)==l_False);
						assert(dbg_value(l)==l_False);
						conflict.push(l);
						buildValueLEQReason(bvID,comparisonID,conflict);
						toSolver(conflict);
						return false;
					}else {
						std::cout<<"propagate bv " << bvID << "<= " << leq << "\n";
						assert(value(l)==l_Undef);
						enqueue(l, comparisonprop_marker);
					}

				}else{
					//break;
				}
			}

			//update under approx lits
			for(int i = compares_leq.size()-1;i>=0;i--){
				int comparisonID = compares_leq[i];
				Weight & leq = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (underApprox>leq){
					if(value(l)==l_True){
						std::cout<<"conflict neg bv " << bvID << "<= " << leq << "\n";
						conflict.push(~l);
						buildValueGTReason(bvID,comparisonID,conflict);
						toSolver(conflict);
						return false;
					}else if (value(l)==l_False){
						//do nothing
						std::cout<<"nothing neg bv " << bvID << "<= " << leq << "\n";
					}else {
						std::cout<<"propagate neg bv " << bvID << "<= " << leq << "\n";
						assert(value(l)==l_Undef);
						enqueue(~l, comparisonprop_marker);
					}

				}else{
					//break;
				}
			}

			vec<int> & compares_gt = comparisons_gt[bvID];
			//update over approx lits
			for(int i = 0;i<compares_gt.size();i++){
				int comparisonID = compares_gt[i];
				Weight & gt = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (overApprox<=gt){
					if(value(l)==l_True){
						std::cout<<"conflict neg bv " << bvID << "> " << gt << "\n";
						conflict.push(~l);
						buildValueLEQReason(bvID,comparisonID,conflict);
						toSolver(conflict);
						return false;
					}else if (value(l)==l_False){
						std::cout<<"nothing neg bv " << bvID << "> " << gt << "\n";
					}else {
						std::cout<<"propagate neg bv " << bvID << "> " << gt << "\n";
						assert(value(l)==l_Undef);
						enqueue(~l, comparisonprop_marker);
					}

				}else{
					//break;
				}
			}

			//update under approx lits
			for(int i = compares_gt.size()-1;i>=0;i--){
				int comparisonID = compares_gt[i];
				Weight & gt = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (underApprox>gt){
					if(value(l)==l_True){
						std::cout<<"nothing bv " << bvID << "> " << gt << "\n";
					}else if (value(l)==l_False){
						std::cout<<"conflict bv " << bvID << "> " << gt << "\n";
						conflict.push(l);
						buildValueGTReason(bvID,comparisonID,conflict);
						toSolver(conflict);
						return false;
					}else {
						std::cout<<"propagate bv " << bvID << "> " << gt << "\n";
						assert(value(l)==l_Undef);
						enqueue(l, comparisonprop_marker);
					}

				}else{
					//break;
				}
			}

			vec<int> & compares_geq = comparisons_geq[bvID];
			//update over approx lits
			for(int i = 0;i<compares_geq.size();i++){
				int comparisonID = compares_geq[i];
				Weight & geq = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (overApprox<geq){
					if(value(l)==l_True){
						std::cout<<"conflict neg bv " << bvID << ">= " << geq << "\n";
						conflict.push(~l);
						buildValueLTReason(bvID,comparisonID,conflict);
						toSolver(conflict);
						return false;
					}else if (value(l)==l_False){
						std::cout<<"nothing neg bv " << bvID << ">= " << geq << "\n";
					}else {
						std::cout<<"propagate neg bv " << bvID << ">= " << geq << "\n";
						assert(value(l)==l_Undef);
						enqueue(~l, comparisonprop_marker);
					}

				}else{
					//break;
				}
			}

			//update under approx lits
			for(int i = compares_geq.size()-1;i>=0;i--){
				int comparisonID = compares_geq[i];
				Weight & geq = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (underApprox>=geq){
					if(value(l)==l_True){
						std::cout<<"nothing bv " << bvID << ">= " << geq << "\n";
					}else if (value(l)==l_False){
						std::cout<<"conflict bv " << bvID << ">= " << geq << "\n";
						conflict.push(l);
						buildValueGEQReason(bvID,comparisonID,conflict);
						toSolver(conflict);
						return false;
					}else {
						std::cout<<"propagate bv " << bvID << ">= " << geq << "\n";
						assert(value(l)==l_Undef);
						enqueue(l, comparisonprop_marker);
					}

				}else{
					//break;
				}
			}

			if(hasTheory(bvID))
				getTheory(bvID)->enqueueBV(bvID);//only enqueue the bitvector in the subtheory _after_ it's approximation has been updated!
			//alteredBV[bvID]=false;
		//	altered_bvs.pop();
		}
		


		
		requiresPropagation = false;

		double elapsed = rtime(1) - startproptime;
		propagationtime += elapsed;

		return true;
	};

	void buildValueLTReason(int bvID, int comparisonID, vec<Lit> & conflict){
		printf("lt reason %d %d\n",bvID, comparisonID);
		Weight & lt = getComparison(comparisonID).w;
		Lit l =  getComparison(comparisonID).l;

		vec<Lit> & bv = bitvectors[bvID];
		Weight  over_cur = over_approx[bvID];
		assert(checkApproxUpToDate(bvID));
		assert(over_cur<lt);


		//the reason that the value is less than the weight 'lt' is that the _overapprox_ of the weight is less than lt.
		/*for(int i =0;i<bv.size();i++){
			Lit bl = bv[i];
			if(value(bl)==l_False && level(var(bl))>0){
				Weight bit = 1<<i;
				if(over+bit<lt){
					//then we can skip this bit, because we would still have had a conflict even if it was assigned true.
					over+=bit;
				}else{
					conflict.push(bl);
				}
			}
		}*/
		Weight under =0;
		Weight over=0;
		for(int i = 0;i<bv.size();i++){
			lbool val = value(bv[i]);
			if(val==l_True){
				Weight bit = 1<<i;
				under+=bit;
				over+=bit;
			}else if (val==l_False){

			}else{
				Weight bit = 1<<i;
				over+=bit;
			}
		}
		if (over<lt){
			//then the reason the underapprox is too large is because of the assignment to the bits
			for(int i =0;i<bv.size();i++){
				Lit bl = bv[i];
				if(value(bl)==l_False ){
					Weight bit = 1<<i;
					if(over+bit<lt&& level(var(bl))>0){
						//then we can skip this bit, because we would still have had a conflict even if it was assigned true.
						over+=bit;
					}else{
						assert(value(bl)==l_False);
						conflict.push(bl);
					}
				}
			}
			return;
		}

		for(int cID:comparisons_lt[bvID]){
			if(cID == comparisonID)
				continue;
			Comparison & c = comparisons[cID];
			if(value( c.l)==l_True && over>=c.w){
				over=c.w-1;
			}else if (value(c.l)==l_False && under<c.w){
				under=c.w;
			}
			if (over<lt){
				//this is the reason for the bitvector being too large
				assert(value(~c.l)==l_False);
				conflict.push(~c.l);
				return;
			}
		}
		for(int cID:comparisons_leq[bvID]){
			if(cID == comparisonID)
				continue;
			Comparison & c = comparisons[cID];
			if(value( c.l)==l_True && over>c.w){
				over=c.w;
			}else if (value(c.l)==l_False && under<=c.w){
				under=c.w+1;
			}
			if (over<lt){
				//this is the reason for the bitvector being too large
				assert(value(~c.l)==l_False);
				conflict.push(~c.l);
				return;
			}
		}
		for(int cID:comparisons_gt[bvID]){
			if(cID == comparisonID)
				continue;
			Comparison & c = comparisons[cID];
			if(value( c.l)==l_True && under<=c.w){
				under=c.w+1;
			}else if (value(c.l)==l_False && over>c.w){
				over=c.w;
			}
			if (over<lt){
				//this is the reason for the bitvector being too large
				assert(value(c.l)==l_False);
				conflict.push(c.l);
				return;
			}
		}
		for(int cID:comparisons_geq[bvID]){
			if(cID == comparisonID)
				continue;
			Comparison & c = comparisons[cID];
			if(value( c.l)==l_True && under<c.w){
				under=c.w;
			}else if (value(c.l)==l_False && over>=c.w){
				over=c.w-1;
			}
			if (over<lt){
				//this is the reason for the bitvector being too large
				assert(value(c.l)==l_False);
				conflict.push(c.l);
				return;
			}
		}
	}

	void buildValueGEQReason(int bvID, int comparisonID, vec<Lit> & conflict){
		printf("geq reason %d %d\n",bvID, comparisonID);
		Weight & lt = getComparison(comparisonID).w;
		Lit l =  getComparison(comparisonID).l;

		vec<Lit> & bv = bitvectors[bvID];
		Weight  under_cur = under_approx[bvID];
		assert(checkApproxUpToDate(bvID));
		assert(under_cur>=lt);
		//the reason that the value is less than the weight 'lt' is that the _overapprox_ of the weight is less than lt.
		//the reason for this depends on how the over approximation was computed - that is, whether it is caused by an assignment to a comparison or to the bitvector itself.

		//this analysis is _only_ correct if we have correctly backtracked in the trail to the point where the comparison was assigned.

		Weight under =0;
		Weight over=0;
		for(int i = 0;i<bv.size();i++){
			lbool val = value(bv[i]);
			if(val==l_True){
				Weight bit = 1<<i;
				under+=bit;
				over+=bit;
			}else if (val==l_False){

			}else{
				Weight bit = 1<<i;
				over+=bit;
			}
		}
		if (under>=lt){
			//then the reason the underapprox is too large is because of the assignment to the bits
			for(int i =0;i<bv.size();i++){
				Lit bl = bv[i];
				lbool val = value(bl);
				lbool dbgval = dbg_value(bl);
				if(value(bl)==l_True){
					Weight bit = 1<<i;
					if(under-bit>=lt  && level(var(bl))>0){
						//then we can skip this bit, because we would still have had a conflict even if it was assigned false.
						under-=bit;
					}else{
						assert(value(~bl)==l_False);
						conflict.push(~bl);
					}
				}
			}
			return;
		}

		for(int cID:comparisons_lt[bvID]){
			if(cID == comparisonID)
				continue;
			Comparison & c = comparisons[cID];
			if(value( c.l)==l_True && over>=c.w){
				over=c.w-1;
			}else if (value(c.l)==l_False && under<c.w){
				under=c.w;
			}
			if (under>=lt){
				//this is the reason for the bitvector being too large
				assert(value(c.l)==l_False);
				conflict.push(c.l);
				return;
			}
		}
		for(int cID:comparisons_leq[bvID]){
			if(cID == comparisonID)
				continue;
			Comparison & c = comparisons[cID];
			if(value( c.l)==l_True && over>c.w){
				over=c.w;
			}else if (value(c.l)==l_False && under<=c.w){
				under=c.w+1;
			}
			if (under>=lt){
				//this is the reason for the bitvector being too large
				assert(value(c.l)==l_False);
				conflict.push(c.l);
				return;
			}
		}
		for(int cID:comparisons_gt[bvID]){
			if(cID == comparisonID)
				continue;
			Comparison & c = comparisons[cID];
			if(value( c.l)==l_True && under<=c.w){
				under=c.w+1;
			}else if (value(c.l)==l_False && over>c.w){
				over=c.w;
			}
			if (under>=lt){
				//this is the reason for the bitvector being too large
				assert(value(~c.l)==l_False);
				conflict.push(~c.l);
				return;
			}
		}
		for(int cID:comparisons_geq[bvID]){
			if(cID == comparisonID)
				continue;
			Comparison & c = comparisons[cID];
			if(value( c.l)==l_True && under<c.w){
				under=c.w;
			}else if (value(c.l)==l_False && over>=c.w){
				over=c.w-1;
			}
			if (under>=lt){
				//this is the reason for the bitvector being too large
				assert(value(~c.l)==l_False);
				conflict.push(~c.l);
				return;
			}
		}


	}

	void buildValueLEQReason(int bvID, int comparisonID, vec<Lit> & conflict){
			printf("leq reason %d %d\n",bvID, comparisonID);
			Weight & lt = getComparison(comparisonID).w;
			Lit l =  getComparison(comparisonID).l;

			vec<Lit> & bv = bitvectors[bvID];
			Weight  over_cur = over_approx[bvID];
			assert(checkApproxUpToDate(bvID));
			assert(over_cur<=lt);


			Weight under =0;
			Weight over=0;
			for(int i = 0;i<bv.size();i++){
				lbool val = value(bv[i]);
				if(val==l_True){
					Weight bit = 1<<i;
					under+=bit;
					over+=bit;
				}else if (val==l_False){

				}else{
					Weight bit = 1<<i;
					over+=bit;
				}
			}
			if (over<=lt){
				//then the reason the underapprox is too large is because of the assignment to the bits
				for(int i =0;i<bv.size();i++){
					Lit bl = bv[i];
					if(value(bl)==l_False ){
						Weight bit = 1<<i;
						if(over+bit<=lt&& level(var(bl))>0){
							//then we can skip this bit, because we would still have had a conflict even if it was assigned true.
							over+=bit;
						}else{
							assert(value(bl)==l_False);
							conflict.push(bl);
						}
					}
				}
				return;
			}

			for(int cID:comparisons_lt[bvID]){
				if(cID == comparisonID)
					continue;
				Comparison & c = comparisons[cID];
				if(value( c.l)==l_True && over>=c.w){
					over=c.w-1;
				}else if (value(c.l)==l_False && under<c.w){
					under=c.w;
				}
				if (over<=lt){
					//this is the reason for the bitvector being too large
					assert(value(~c.l)==l_False);
					conflict.push(~c.l);
					return;
				}
			}
			for(int cID:comparisons_leq[bvID]){
				if(cID == comparisonID)
					continue;
				Comparison & c = comparisons[cID];
				if(value( c.l)==l_True && over>c.w){
					over=c.w;
				}else if (value(c.l)==l_False && under<=c.w){
					under=c.w+1;
				}
				if (over<=lt){
					//this is the reason for the bitvector being too large
					assert(value(~c.l)==l_False);
					conflict.push(~c.l);
					return;
				}
			}
			for(int cID:comparisons_gt[bvID]){
				if(cID == comparisonID)
					continue;
				Comparison & c = comparisons[cID];
				if(value( c.l)==l_True && under<=c.w){
					under=c.w+1;
				}else if (value(c.l)==l_False && over>c.w){
					over=c.w;
				}
				if (over<=lt){
					//this is the reason for the bitvector being too large
					assert(value(c.l)==l_False);
					conflict.push(c.l);
					return;
				}
			}
			for(int cID:comparisons_geq[bvID]){
				if(cID == comparisonID)
					continue;
				Comparison & c = comparisons[cID];
				if(value( c.l)==l_True && under<c.w){
					under=c.w;
				}else if (value(c.l)==l_False && over>=c.w){
					over=c.w-1;
				}
				if (over<=lt){
					//this is the reason for the bitvector being too large
					assert(value(c.l)==l_False);
					conflict.push(c.l);
					return;
				}
			}
		}

		void buildValueGTReason(int bvID, int comparisonID, vec<Lit> & conflict){
			printf("gt reason %d %d\n",bvID, comparisonID);
			Weight & lt = getComparison(comparisonID).w;
			Lit l =  getComparison(comparisonID).l;

			vec<Lit> & bv = bitvectors[bvID];
			Weight  under_cur = under_approx[bvID];
			assert(checkApproxUpToDate(bvID));
			assert(under_cur>lt);
			//the reason that the value is less than the weight 'lt' is that the _overapprox_ of the weight is less than lt.

		/*	for(int i =0;i<bv.size();i++){
				Lit bl = bv[i];
				if(value(bl)==l_True && level(var(bl))>0){
					Weight bit = 1<<i;
					if(under-bit>lt){
						//then we can skip this bit, because we would still have had a conflict even if it was assigned false.
						under-=bit;
					}else{
						conflict.push(~bl);
					}
				}
			}*/

			Weight under =0;
			Weight over=0;
			for(int i = 0;i<bv.size();i++){
				lbool val = value(bv[i]);
				if(val==l_True){
					Weight bit = 1<<i;
					under+=bit;
					over+=bit;
				}else if (val==l_False){

				}else{
					Weight bit = 1<<i;
					over+=bit;
				}
			}
			if (under>lt){
				//then the reason the underapprox is too large is because of the assignment to the bits
				for(int i =0;i<bv.size();i++){
					Lit bl = bv[i];
					lbool val = value(bl);
					lbool dbgval = dbg_value(bl);
					if(value(bl)==l_True){
						Weight bit = 1<<i;
						if(under-bit>lt  && level(var(bl))>0){
							//then we can skip this bit, because we would still have had a conflict even if it was assigned false.
							under-=bit;
						}else{
							assert(value(~bl)==l_False);
							conflict.push(~bl);
						}
					}
				}
				return;
			}

			for(int cID:comparisons_lt[bvID]){
				if(cID == comparisonID)
					continue;
				Comparison & c = comparisons[cID];
				if(value( c.l)==l_True && over>=c.w){
					over=c.w-1;
				}else if (value(c.l)==l_False && under<c.w){
					under=c.w;
				}
				if (under>lt){
					//this is the reason for the bitvector being too large
					assert(value(c.l)==l_False);
					conflict.push(c.l);
					return;
				}
			}
			for(int cID:comparisons_leq[bvID]){
				if(cID == comparisonID)
					continue;
				Comparison & c = comparisons[cID];
				if(value( c.l)==l_True && over>c.w){
					over=c.w;
				}else if (value(c.l)==l_False && under<=c.w){
					under=c.w+1;
				}
				if (under>lt){
					//this is the reason for the bitvector being too large
					assert(value(c.l)==l_False);
					conflict.push(c.l);
					return;
				}
			}
			for(int cID:comparisons_gt[bvID]){
				if(cID == comparisonID)
					continue;
				Comparison & c = comparisons[cID];
				if(value( c.l)==l_True && under<=c.w){
					under=c.w+1;
				}else if (value(c.l)==l_False && over>c.w){
					over=c.w;
				}
				if (under>lt){
					//this is the reason for the bitvector being too large
					assert(value(~c.l)==l_False);
					conflict.push(~c.l);
					return;
				}
			}
			for(int cID:comparisons_geq[bvID]){
				if(cID == comparisonID)
					continue;
				Comparison & c = comparisons[cID];
				if(value( c.l)==l_True && under<c.w){
					under=c.w;
				}else if (value(c.l)==l_False && over>=c.w){
					over=c.w-1;
				}
				if (under>lt){
					//this is the reason for the bitvector being too large
					assert(value(~c.l)==l_False);
					conflict.push(~c.l);
					return;
				}
			}
		}

	bool solveTheory(vec<Lit> & conflict) {
		requiresPropagation = true;		//Just to be on the safe side... but this shouldn't really be required.
		bool ret = propagateTheory(conflict);
		//Under normal conditions, this should _always_ hold (as propagateTheory should have been called and checked by the parent solver before getting to this point).
		assert(ret);
		return ret;
	}

	bool check_solved() {
		for(int bvID = 0;bvID<bitvectors.size();bvID++){
			vec<Lit> & bv = bitvectors[bvID];
			Weight over=0;
			Weight under=0;
			for(int i = 0;i<bv.size();i++){
				lbool val = value(bv[i]);
				if(val==l_True){
					Weight bit = 1<<i;
					under+=bit;
					over+=bit;
				}else if (val==l_False){

				}else{
					Weight bit = 1<<i;
					over+=bit;
				}
			}
			for(int cID:comparisons_lt[bvID]){
				Comparison & c = comparisons[cID];
				if(value(c.l)==l_True && under>= c.w){
					return false;
				}else if (value(c.l)==l_False && over<c.w){
					return false;
				}
			}

			for(int cID:comparisons_gt[bvID]){
				Comparison & c = comparisons[cID];
				if(value(c.l)==l_True && over<= c.w){
					return false;
				}else if (value(c.l)==l_False && under>c.w){
					return false;
				}
			}

		}
		return true;
	}
	
	bool dbg_solved() {

		return true;
	}




	void setBitvectorTheory(int bvID, int theoryID){
		theoryIds[bvID]=theoryID;
	}

	BitVector newBitvector(int bvID, vec<Var> & vars){
		if(bvID<0){
			bvID = bitvectors.size();
		}
		//bv_callbacks.growTo(id+1,nullptr);
		bitvectors.growTo(bvID+1);
		theoryIds.growTo(bvID+1,-1);
		in_backtrack_queue.growTo(bvID+1,-1);
		under_approx.growTo(bvID+1,-1);
		over_approx.growTo(bvID+1,-1);
		comparisons_lt.growTo(bvID+1);
		comparisons_gt.growTo(bvID+1);
		comparisons_leq.growTo(bvID+1);
		comparisons_geq.growTo(bvID+1);
		alteredBV.growTo(bvID+1);
		//bv_callbacks.growTo(bvID+1);
		if(under_approx[bvID]>-1){
			assert(false);
			fprintf(stderr,"Redefined bitvector, Aborting!");
			exit(1);
		}
		under_approx[bvID]=0;
		over_approx[bvID]=0;

		for(int i = 0;i<vars.size();i++){
			bitvectors[bvID].push(mkLit(newVar(vars[i],bvID)));
		}

		alteredBV[bvID]=true;
		altered_bvs.push(bvID);
		return BitVector(*this,bvID);
	}


	BitVector newBitvector(int bvID, int bitwidth){
		if(bvID<0){
			bvID = bitvectors.size();
		}
		//bv_callbacks.growTo(id+1,nullptr);
		bitvectors.growTo(bvID+1);
		theoryIds.growTo(bvID+1,-1);
		in_backtrack_queue.growTo(bvID+1,-1);
		under_approx.growTo(bvID+1,-1);
		over_approx.growTo(bvID+1,-1);
		comparisons_lt.growTo(bvID+1);
		comparisons_gt.growTo(bvID+1);
		comparisons_leq.growTo(bvID+1);
		comparisons_geq.growTo(bvID+1);
		alteredBV.growTo(bvID+1);


		//bv_callbacks.growTo(bvID+1);
		if(under_approx[bvID]>-1){
			assert(false);
			fprintf(stderr,"Redefined bitvector, Aborting!");
			exit(1);
		}
		under_approx[bvID]=0;
		over_approx[bvID]=0;

		for(int i = 0;i<bitwidth;i++){
			bitvectors[bvID].push(mkLit(newVar(var_Undef,bvID)));
		}

		alteredBV[bvID]=true;
		altered_bvs.push(bvID);
		return BitVector(*this,bvID);
	}


	bool hasBV(int bvID){
		return bvID>=0 && bvID<under_approx.size() && under_approx[bvID]>-1;
	}
private:
	Lit getComparisonLT(int bvID,const Weight & lt){
		//could do a binary search here:
		for(int i=0;i<comparisons_lt[bvID].size();i++){
			int cID = comparisons_lt[bvID][i];
			if (comparisons[cID].w == lt){
				return comparisons[cID].l;
			}
		}

		return lit_Undef;
	}
	Lit getComparisonGT(int bvID,const Weight & gt){
		//could do a binary search here:
		for(int i=0;i<comparisons_gt[bvID].size();i++){
			int cID = comparisons_gt[bvID][i];
			if (comparisons[cID].w == gt){
				return comparisons[cID].l;
			}
		}

		return lit_Undef;
	}
	Lit getComparisonLEQ(int bvID,const Weight & leq){
		//could do a binary search here:
		vec<int>  & comparison_leq = comparisons_leq[bvID];
		for(int i=0;i<comparison_leq.size();i++){
			int cID = comparison_leq[i];
			if (comparisons[cID].w == leq){
				return comparisons[cID].l;
			}
		}

		return lit_Undef;
	}
	Lit getComparisonGEQ(int bvID,const Weight & geq){
		//could do a binary search here:
		for(int i=0;i<comparisons_geq[bvID].size();i++){
			int cID = comparisons_geq[bvID][i];
			if (comparisons[cID].w == geq){
				return comparisons[cID].l;
			}
		}

		return lit_Undef;
	}
public:
	Lit newComparisonLT(int bvID,const Weight & lt, Var outerVar = var_Undef) {
		Lit l;
		if(!hasBV(bvID)){
			exit(1);
		}
		int comparisonID = comparisons.size();
		if(comparisonID==7){
			int a=1;
		}
		if((l = getComparisonLT(bvID, lt))!=lit_Undef){
			if(outerVar != var_Undef){
				makeEqualInSolver(mkLit(outerVar),toSolver(l));
			}
			return l;
		}else{
			l = mkLit(newVar(outerVar, bvID,comparisonID));
		}
		std::cout<<"New comparison " << comparisonID << ": bv"<< bvID << " < " << lt <<"\n";
		updateApproximations(bvID);
		comparisons.push({lt,l,bvID,true,true});
		comparisons_lt[bvID].push(comparisonID);
		//insert this value in order.
		//could do a binary search here...
		for(int i=0;i<comparisons_lt[bvID].size()-1;i++){
			int cid = comparisons_lt[bvID][i];
			if(comparisons[cid].w>= lt){
				for(int j = comparisons_lt[bvID].size()-1; j>i ;j--){
					comparisons_lt[bvID][j]=comparisons_lt[bvID][j-1];
				}
				comparisons_lt[bvID][i]=comparisonID;
				break;
			}
		}
		if(!alteredBV[bvID]){
			alteredBV[bvID]=true;
			altered_bvs.push(bvID);
		}
		//set the value of this immediately, if needed

		Weight & underApprox = under_approx[bvID];
		Weight & overApprox = over_approx[bvID];


		if (overApprox<lt){
			if(value(l)==l_True){
				//do nothing
			}else if (value(l)==l_False){
				assert(false);//this should not happen!
			}else {
				assert(value(l)==l_Undef);

				enqueueEager(l, comparisonprop_marker);
			}
		}

		if (underApprox>=lt){
			if(value(l)==l_True){
				assert(false);
			}else if (value(l)==l_False){
				//do nothing
			}else {
				assert(value(l)==l_Undef);
				enqueueEager(~l, comparisonprop_marker);
			}
		}

		return l;
	}
	Lit newComparisonLEQ(int bvID,const Weight & leq, Var outerVar = var_Undef) {
			Lit l;
			if(!hasBV(bvID)){
				exit(1);
			}
			int comparisonID = comparisons.size();
			if(comparisonID==7){
				int a=1;
			}
			if((l = getComparisonLEQ(bvID, leq))!=lit_Undef){
				if(outerVar != var_Undef){
					makeEqualInSolver(mkLit(outerVar),toSolver(l));
				}
				return l;
			}else{
				l = mkLit(newVar(outerVar, bvID,comparisonID));
			}
			updateApproximations(bvID);
			std::cout<<"New comparison " << comparisonID << ": bv"<< bvID << " <= " << leq <<"\n";
			//std::cout<<"constraint: bv " << bvID << "<= " << leq << "\n";
			comparisons.push({leq,l,bvID,true,false});
			comparisons_leq[bvID].push(comparisonID);
			//insert this value in order.
			//could do a binary search here...
			for(int i=0;i<comparisons_leq[bvID].size()-1;i++){
				int cid = comparisons_leq[bvID][i];
				if(comparisons[cid].w>= leq){
					for(int j = comparisons_leq[bvID].size()-1; j>i ;j--){
						comparisons_leq[bvID][j]=comparisons_leq[bvID][j-1];
					}
					comparisons_leq[bvID][i]=comparisonID;
					break;
				}
			}
			if(!alteredBV[bvID]){
				alteredBV[bvID]=true;
				altered_bvs.push(bvID);
			}
			//set the value of this immediately, if needed

			Weight & underApprox = under_approx[bvID];
			Weight & overApprox = over_approx[bvID];
			assert(under_approx[bvID]<=overApprox);

			if (overApprox<=leq){
				if(value(l)==l_True){
					//do nothing
				}else if (value(l)==l_False){
					assert(false);//this should not happen!
				}else {
					assert(value(l)==l_Undef);
					enqueueEager(l, comparisonprop_marker);
				}
			}

			if (underApprox>leq){
				if(value(l)==l_True){
					assert(false);
				}else if (value(l)==l_False){
					//do nothing
				}else {
					assert(value(l)==l_Undef);
					enqueueEager(~l, comparisonprop_marker);
				}
			}

			return l;
		}
	Lit newComparisonGT(int bvID,const Weight & gt, Var outerVar = var_Undef) {
		Lit l;
		int comparisonID = comparisons.size();
		if((l = getComparisonGT(bvID, gt))!=lit_Undef){
			if(outerVar != var_Undef){
				makeEqualInSolver(mkLit(outerVar),toSolver(l));
			}
			return l;
		}else{
			l = mkLit(newVar(outerVar, bvID,comparisonID));
		}

		std::cout<<"New comparison " << comparisonID << ": bv"<< bvID << " > " << gt <<"\n";
		updateApproximations(bvID);
		comparisons.push({gt,l,bvID,false,true});
		comparisons_gt[bvID].push(comparisonID);
		//insert this value in order.
		//could do a binary search here...
		for(int i=0;i<comparisons_gt[bvID].size()-1;i++){
			int cid = comparisons_gt[bvID][i];
			if(comparisons[cid].w>= gt){
				for(int j = comparisons_gt[bvID].size()-1; j>i ;j--){
					comparisons_gt[bvID][j]=comparisons_gt[bvID][j-1];
				}
				comparisons_gt[bvID][i]=comparisonID;
				break;
			}
		}
		if(!alteredBV[bvID]){
			alteredBV[bvID]=true;
			altered_bvs.push(bvID);
		}



		Weight & underApprox = under_approx[bvID];
		Weight & overApprox = over_approx[bvID];


		if (overApprox<=gt){
			if(value(l)==l_True){
				assert(false);
			}else if (value(l)==l_False){

			}else {
				assert(value(l)==l_Undef);
				enqueueEager(~l, comparisonprop_marker);
			}
		}

		if (underApprox>gt){
			if(value(l)==l_True){

			}else if (value(l)==l_False){
				assert(false);
			}else {
				assert(value(l)==l_Undef);
				enqueueEager(l, comparisonprop_marker);
			}
		}



		return l;
	}

	Lit newComparisonGEQ(int bvID,const Weight & geq, Var outerVar = var_Undef) {
			Lit l;
			int comparisonID = comparisons.size();
			if((l = getComparisonGEQ(bvID, geq))!=lit_Undef){
				if(outerVar != var_Undef){
					makeEqualInSolver(mkLit(outerVar),toSolver(l));
				}
				return l;
			}else{
				l = mkLit(newVar(outerVar, bvID,comparisonID));
			}

			std::cout<<"New comparison " << comparisonID << ": bv"<< bvID << " >= " << geq <<"\n";
			updateApproximations(bvID);
			comparisons.push({geq,l,bvID,false,false});
			comparisons_geq[bvID].push(comparisonID);
			//insert this value in order.
			//could do a binary search here...
			for(int i=0;i<comparisons_geq[bvID].size()-1;i++){
				int cid = comparisons_geq[bvID][i];
				if(comparisons[cid].w>= geq){
					for(int j = comparisons_geq[bvID].size()-1; j>i ;j--){
						comparisons_geq[bvID][j]=comparisons_geq[bvID][j-1];
					}
					comparisons_geq[bvID][i]=comparisonID;
					break;
				}
			}
			if(!alteredBV[bvID]){
				alteredBV[bvID]=true;
				altered_bvs.push(bvID);
			}


			Weight & underApprox = under_approx[bvID];
			Weight & overApprox = over_approx[bvID];


			if (overApprox<geq){
				if(value(l)==l_True){
					assert(false);
				}else if (value(l)==l_False){

				}else {
					assert(value(l)==l_Undef);
					enqueueEager(~l, comparisonprop_marker);
				}
			}

			if (underApprox>=geq){
				if(value(l)==l_True){

				}else if (value(l)==l_False){
					assert(false);
				}else {
					assert(value(l)==l_Undef);
					enqueueEager(l, comparisonprop_marker);
				}
			}


			return l;
		}

	void printSolution() {

	}

	bool dbg_uptodate(){
#ifndef NDEBUG
		dbg_synced();
		for(int bvID = 0;bvID<bitvectors.size();bvID++){
			assert(checkApproxUpToDate(bvID));

			Weight & underApprox = under_approx[bvID];
			Weight & overApprox = over_approx[bvID];
			vec<int> & compares_lt = comparisons_lt[bvID];
			//update over approx lits
			for(int i = 0;i<compares_lt.size();i++){
				int comparisonID = compares_lt[i];
				Weight & lt = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (overApprox<lt){
					if(value(l)==l_True){
						//do nothing
					}else if (value(l)==l_False){
						assert(false);
						return false;
					}else {
						assert(false);
						return false;
					}

				}else{
					break;
				}
			}

			//update under approx lits
			for(int i = compares_lt.size()-1;i>=0;i--){
				int comparisonID = compares_lt[i];
				Weight & lt = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (underApprox>=lt){
					if(value(l)==l_True){
						assert(false);
						return false;
					}else if (value(l)==l_False){
						//do nothing
					}else {
						assert(false);
						return false;
					}

				}else{
					break;
				}
			}

			vec<int> & compares_leq = comparisons_leq[bvID];
			//update over approx lits
			for(int i = 0;i<compares_leq.size();i++){
				int comparisonID = compares_leq[i];
				Weight & leq = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (overApprox<=leq){
					if(value(l)==l_True){

					}else if (value(l)==l_False){
						assert(false);
						return false;
					}else {
						assert(false);
						return false;
					}

				}else{
					//break;
				}
			}

			//update under approx lits
			for(int i = compares_leq.size()-1;i>=0;i--){
				int comparisonID = compares_leq[i];
				Weight & leq = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (underApprox>leq){
					if(value(l)==l_True){
						assert(false);
						return false;
					}else if (value(l)==l_False){

					}else {
						assert(false);
						return false;
					}

				}else{
					//break;
				}
			}

			vec<int> & compares_gt = comparisons_gt[bvID];
			//update over approx lits
			for(int i = 0;i<compares_gt.size();i++){
				int comparisonID = compares_gt[i];
				Weight & gt = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (overApprox<=gt){
					if(value(l)==l_True){
						assert(false);
						return false;
					}else if (value(l)==l_False){

					}else {
						assert(false);
						return false;
					}

				}else{
					break;
				}
			}

			//update under approx lits
			for(int i = compares_gt.size()-1;i>=0;i--){
				int comparisonID = compares_gt[i];
				Weight & gt = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (underApprox>gt){
					if(value(l)==l_True){

					}else if (value(l)==l_False){
						assert(false);
						return false;
					}else {
						assert(false);
						return false;
					}

				}else{
					break;
				}
			}

			vec<int> & compares_geq = comparisons_geq[bvID];
			//update over approx lits
			for(int i = 0;i<compares_geq.size();i++){
				int comparisonID = compares_geq[i];
				Weight & geq = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (overApprox<geq){
					if(value(l)==l_True){
						assert(false);
						return false;
					}else if (value(l)==l_False){

					}else {
						assert(false);
						return false;
					}

				}else{
					break;
				}
			}

			//update under approx lits
			for(int i = compares_geq.size()-1;i>=0;i--){
				int comparisonID = compares_geq[i];
				Weight & geq = getComparison(comparisonID).w;
				Lit l =  getComparison(comparisonID).l;
				if (underApprox>=geq){
					if(value(l)==l_True){

					}else if (value(l)==l_False){
						assert(false);
						return false;
					}else {
						assert(false);
						return false;
					}

				}else{
					break;
				}
			}


		}
#endif
		return true;
	}

};
template<typename Weight>
using BitVector = typename BVTheorySolver<Weight>::BitVector;

}
;

#endif
