/*
 * Theory.h
 *
 *  Created on: 2013-04-14
 *      Author: sam
 */

#ifndef THEORY_H_
#define THEORY_H_

#include "mtl/Vec.h"
#include "mtl/Heap.h"
#include "mtl/Alg.h"
#include "utils/Options.h"
#include "core/SolverTypes.h"
#include "Solver.h"

namespace Minisat{
class Solver;
class Theory{
public:
    virtual ~Theory(){};
	virtual void backtrackUntil(int level)=0;
	virtual void newDecisionLevel()=0;
	virtual bool propagate(vec<Lit> & conflict)=0;
	virtual bool solve(vec<Lit> & conflict)=0;
	//Lazily construct the reason clause explaining this propagation
	virtual void buildReason(Lit p, vec<Lit> & reason)=0;
};
}

#endif /* THEORY_H_ */