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

#ifndef GEOMETRY_PARSER_H_
#define GEOMETRY_PARSER_H_

#include <stdio.h>
#include <gmpxx.h>
#include "monosat/utils/ParseUtils.h"
#include "monosat/graph/GraphParser.h"
#include "monosat/geometry/GeometryTypes.h"
#include "monosat/core/SolverTypes.h"
#include "monosat/geometry/GeometryTheory.h"
#include "monosat/core/Dimacs.h"
#include "monosat/core/Config.h"
#include "monosat/mtl/Vec.h"
#include <vector>
#include <map>

namespace Monosat {

// GEOMETRY Parser:
template<class B, class Solver>
class GeometryParser: public Parser<B, Solver> {
	using Parser<B, Solver>::mapVar;
	vec<GeometryTheorySolver<2, int>*> geometryTheories;
	// These exist to translate from the global name space to the theory's local name space
	vec<map<int,int>> pointNameMap;
	vec<map<int,int>> planeNameMap;
	vec<map<int,int>> shapeNameMap;

	void readCSG(B& in, Solver& S) {
		if (opt_ignore_theories) {
			skipLine(in);
			return;
		}
		
		if (match(in, "int")) {
			// We'll do stuff here later.
		} else if (match(in, "float")) {
			// We'll do stuff here later.
		} else if (match(in, "rational")) {
			// We'll do stuff here later.
		} else {
			// We'll do stuff here later.
		}

		int d, name;
		d = parseInt(in); // We'll use this later
		name = parseInt(in);
		geometryTheories.growTo(name + 1);
		shapeNameMap.growTo(name + 1);
		planeNameMap.growTo(name + 1);
		pointNameMap.growTo(name + 1);

		GeometryTheorySolver<2, int> *geo = new GeometryTheorySolver<2, int>(&S);

		geometryTheories[name] = geo;
		S.addTheory(geo);
	}

	void readPoint(B& in, Solver& S) {
		if (opt_ignore_theories) {
			skipLine(in);
			return;
		}
		
		
		int csgName, dim, name;
		
		csgName = parseInt(in); 

		if (match(in, "int")) {
			// We'll do stuff here later.
		} else if (match(in, "float")) {
			// We'll do stuff here later.
		} else if (match(in, "rational")) {
			// We'll do stuff here later.
		} else {
			// We'll do stuff here later.
		}
		vec<int> point;
		dim = 2;
		point.growTo(dim);

		for (int i = 0; i < dim; i++) {
			point[i] = parseInt(in);
		}
		name = parseInt(in); 

		pointNameMap[csgName][name] = geometryTheories[csgName]->addPoint(&point);
	}

	void readPlane(B& in, Solver& S) {
		if (opt_ignore_theories) {
			skipLine(in);
			return;
		}
		
		int csgName, point, normalVector, name;
		
		csgName = parseInt(in); 

		if (match(in, "int")) {
			// We'll do stuff here later.
		} else if (match(in, "float")) {
			// We'll do stuff here later.
		} else if (match(in, "rational")) {
			// We'll do stuff here later.
		} else {
			// We'll do stuff here later.
		}

		point = parseInt(in); 
		normalVector = parseInt(in); 
		name = parseInt(in); 

		int localPointName = pointNameMap[csgName][point];
		int localVectorName = pointNameMap[csgName][normalVector];

		planeNameMap[csgName][name] = geometryTheories[csgName]->addPlane(localPointName, localVectorName);
	}

	void readPrimative(B& in, Solver& S) {
		if (opt_ignore_theories) {
			skipLine(in);
			return;
		}
		
		int csgName, plane, name, varName;
		
		csgName = parseInt(in); 

		if (match(in, "int")) {
			// We'll do stuff here later.
		} else if (match(in, "float")) {
			// We'll do stuff here later.
		} else if (match(in, "rational")) {
			// We'll do stuff here later.
		} else {
			// We'll do stuff here later.
		}

		vec<int> localPrimative;
		int size = parseInt(in); 
		localPrimative.growTo(size);
		for (int i = 0; i < size; i++) {
			plane = parseInt(in);
			localPrimative[i] = planeNameMap[csgName][plane];
		}

		name = parseInt(in); 
		varName = parseInt(in) - 1; 

		if (varName == -1) {
			shapeNameMap[csgName][name] = geometryTheories[csgName]->addPrimative(&localPrimative);
		} else {
			varName = mapVar(S,varName);
			shapeNameMap[csgName][name] = geometryTheories[csgName]->addConditionalPrimative(&localPrimative, varName);
		}
	}

	void readShape(B& in, Solver& S, int type) {
		if (opt_ignore_theories) {
			skipLine(in);
			return;
		}
		
		int csgName, left, right, name, varName;
		
		csgName = parseInt(in); 

		if (match(in, "int")) {
			// We'll do stuff here later.
		} else if (match(in, "float")) {
			// We'll do stuff here later.
		} else if (match(in, "rational")) {
			// We'll do stuff here later.
		} else {
			// We'll do stuff here later.
		}

		left = parseInt(in); 
		left = shapeNameMap[csgName][left];
		right = parseInt(in); 
		right = shapeNameMap[csgName][right];
		name = parseInt(in); 
		varName = parseInt(in); 
		if (varName == -1) {
			shapeNameMap[csgName][name] = geometryTheories[csgName]->addShape(left, right, type);
		} else {
			varName = mapVar(S,varName);
			shapeNameMap[csgName][name] = geometryTheories[csgName]->addConditionalShape(left, right, type, varName);
		}
	}

	void readPointContains(B& in, Solver& S) {
		if (opt_ignore_theories) {
			skipLine(in);
			return;
		}
		
		int csgName, point, shape, varName;
		
		csgName = parseInt(in); 

		if (match(in, "int")) {
			// We'll do stuff here later.
		} else if (match(in, "float")) {
			// We'll do stuff here later.
		} else if (match(in, "rational")) {
			// We'll do stuff here later.
		} else {
			// We'll do stuff here later.
		}

		point = parseInt(in); 
		point = pointNameMap[csgName][point];
		shape = parseInt(in); 
		shape = shapeNameMap[csgName][shape];
		varName = parseInt(in); 
		varName = mapVar(S,varName);
		geometryTheories[csgName]->addShapeContainsPoint(shape, point, varName);
	}
	
public:
	
	GeometryParser():Parser<B, Solver>("Geometry"){

	}


	bool parseLine(B& in, Solver& S) {
		
		skipWhitespace(in);
		if (*in == EOF) {
			return false;
		} else if (match(in, "geometry")) {
			readCSG(in, S);
		} else if (match(in, "point")) {
			readPoint(in, S);
		} else if (match(in, "plane")) {
			readPlane(in, S);
		} else if (match(in, "primative")) {
			readPrimative(in, S);
		} else if (match(in, "union")) {
			readShape(in, S, 0);
		} else if (match(in, "intersection")) {
			readShape(in, S, 1);
		} else if (match(in, "difference")) {
			readShape(in, S, 2);
		} else if (match(in, "predicate_point_in_shape")) {
			readPointContains(in, S);
		} else {
			return false;
		}
		return true;
	}
	
	void implementConstraints(Solver & S) {
		return;
	}
	
};

//=================================================================================================
}
;

#endif /* GRAPH_PARSER_H_ */
