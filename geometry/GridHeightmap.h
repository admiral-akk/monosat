/*
 * GridHeightmap.h
 *
 *  Created on: 2014-02-26
 *      Author: sam
 */
#ifndef GRID_HEIGHTMAP_H
#define GRID_HEIGHTMAP_H

#include "Heightmap.h"
#include "mtl/Vec.h"
#include "PointSet.h"
//A simple heightmap composed of regularly spaced rectangular prisms
namespace Minisat{

template<unsigned int D,class T=double>
class GridHeightmap:public Heightmap<D,T>{
	int width;
	int depth;
	vec<vec<Point<D,T> > > grid;
	Polygon<D,T> heightmesh;
	GridHeightmap(int width_, int depth_):width(width_),depth(depth_){

		for(int i = 0;i<width;i++){
			grid.push();
			vec<Point<D,T>> &v = grid.last();
			for(int j = 0;j<depth;j++){
				v.push({i,j,-1});
			}
		}
	}

	void setPoint(int x, int y, T height){
		assert(x>=0);
		assert(x<width);
		assert(y>=0);
		assert(y<depth);
		grid[x][y]={x,y,height};
	}

	Point<D,T> getPoint(int x, int y){
		return grid[x][y];
	}

	void update();

	Polygon<D,T> & getHeightmesh(){
		return heightmesh;
	}


};
template<>
void GridHeightmap<3,double>::update();
};
#endif