/*
 * Chokepoint.h
 *
 *  Created on: 2013-08-04
 *      Author: sam
 */
#ifndef CHOKEPOINT_H_
#define CHOKEPOINT_H_

#include "DynamicGraph.h"
struct ForceReason{
		int edge_id;
		int node;
	} ;

template <class EdgeStatus,class GraphStatus>
class Chokepoint{

	DynamicGraph<GraphStatus> & g;
	EdgeStatus & status;
	int source;
	vec<int> current;
	vec<int> prev;
	vec<int> queue;
	vec<int> dist;
	vec<bool> in_queue;

	static const int SOURCE = -3;
	static const int UNDEF=-2;
	static const int EMPTY=-1;
public:
	Chokepoint(EdgeStatus & _status,DynamicGraph<GraphStatus> & _graph, int _source):g(_graph),status(_status),source(_source)
	{

	}

	void update(){

		in_queue.clear();
		queue.clear();

		current.growTo(g.nodes);
		prev.growTo(g.nodes);
		dist.growTo(g.nodes);
		in_queue.growTo(g.nodes);



		for(int i = 0;i<g.nodes;i++){
			prev[i]=UNDEF;
			current[i]=UNDEF;
			dist[i]=UNDEF;
		}

		prev[source] = SOURCE;
		current[source]=SOURCE;
		dist[source]=0;

		in_queue[source]= 1;
		queue.push(source);

		for(int i = 0;i<queue.size();i++){
			int u = queue[i];

			int old_dist = dist[u];


			assert(in_queue[u]);


			if(u==source){
				//a better way to model this would be to give the source node an extra incoming edge
			}else{
				in_queue[u]=false;
				current[u]=UNDEF;
						prev[u]=UNDEF;

				for(int j = 0;j<g.inverted_adjacency[u].size();j++){
					int from = g.inverted_adjacency[u][j].node;
					int id= g.inverted_adjacency[u][j].id;
					if(!g.edgeEnabled(id))
							continue;

					//From does not (yet) have a path from source (if it later gets one, then u will be re-added to the queue and re-processed
					if(prev[from]==UNDEF)
						continue;

					if(prev[u]==UNDEF){
						prev[u]=from;
						if(status(id)){
							//then this edge is a candidate edge
							current[u] = id;
						}else{
							current[u] = UNDEF;
						}
						dist[u]=dist[from]+1;

						continue;
					}



					//compare the incoming list to the previous list.
					//First, make the distances equal
					while(dist[u]>dist[from]+1){
						prev[u] = prev[prev[u]];
						dist[u]--;
						assert(dist[i]==dist[prev[u]]+1);
					}

					while(dist[u]>=0 && dist[from]>dist[u]){
						from = prev[from];
					}

					//now, backtrack in both linked lists until they have the same current element
					while(current[u]!= current[from]){
						prev[u] = prev[prev[u]];
						dist[u]--;
						assert(dist[u]>=0);
						from = prev[from];
						assert(dist[i]==dist[prev[u]]+1);
					}

					assert(dist[u]>=0);







				}
			}
				//ok, if we changed anything, update all outgoing edges

				if(dist[u]!= old_dist || u==source){
					assert(dist[u]>=0);


					for(int j = 0;j<g.adjacency[u].size();j++){
						int to = g.adjacency[u][j].node;
						int id= g.adjacency[u][j].id;
						if(!g.edgeEnabled(id))
								continue;

						if(!in_queue[to]){
							queue.push(to);
							in_queue[to]=true;
						}

					}

				}



		}

	}

	void collectForcedEdges(vec<ForceReason> & forced_ids){
		update();
		for(int i = 0;i<g.nodes;i++){
			if(status.mustReach(i)){
				int u= i;
				while(prev[u]>=0){
					if(current[u]!=UNDEF){
						forced_ids.push({current[u],i});
						current[u]=UNDEF;
					}
					int v = prev[u];
					prev[u]=UNDEF;
					u=v;
				}
			}
		}
	}


};


#endif


