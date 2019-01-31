#include <algorithm>
#include "path.h"

bool nodeCompare(PathNode node1, PathNode node2) {
	if (node1.devWeight > node2.devWeight) return true;
	else if (node1.devWeight < node2.devWeight) return false;
	else return node1.node > node2.node;
}

bool edgeCompare(PathEdge edge1, PathEdge edge2) {
	if (edge1.dist > edge2.dist) return true;
	else if (edge1.dist < edge2.dist) return false;
	else return edge1.node1 > edge2.node1;
}

Path::Path(const Instance& instance, vector<Node> path) :
instance(instance),
path(path)
{
	isValid = true;
	dist = 0;
	weight = 0;
	length = path.size();

	for (unsigned int i = 0; i < path.size()-1; ++i) {
		if (!instance.hasEdge(path[i],path[i+1])) {
			isValid = false;
			break;
		}

		int d = instance.edgeDist(path[i],path[i+1]);
		PathEdge edge = {path[i],path[i+1],d,0.};
		edges.push_back(edge);
		dist += d;

		int w = instance.nodeWeight(path[i]);
		PathNode node = {path[i],w,(float) instance.ph[path[i]],0.};
		nodes.push_back(node);
		weight += w;
	}

	int w = instance.nodeWeight(path.back());
	PathNode node = {path.back(),w,(float) instance.ph[path.back()],0.};
	nodes.push_back(node);
	weight += w;

	std::sort(nodes.begin(),nodes.end(),nodeCompare);
	std::sort(edges.begin(),edges.end(),edgeCompare);

	float d1 = (float) instance.d1;
	for (unsigned int i = 0; i < edges.size(); ++i) {
		float D = instance.D[edges[i].node1][edges[i].node2];
		if (d1 > D) {
			edges[i].dev = D;
			d1 -= D;
		}
		else {
			edges[i].dev = d1;
			d1 = 0;
			break;
		}
	}

	float d2 = (float) instance.d2;
	for (unsigned int i = 0; i < nodes.size(); ++i) {
		if (d2 > 2.) {
			nodes[i].dev = 2.;
			d2 -= 2.;
		}
		else {
			nodes[i].dev = d2;
			d2 = 0.;
			break;
		}
	}

	worstDist = dist;
	for (unsigned int i = 0; i < edges.size(); ++i) {
		worstDist += edges[i].dist * edges[i].dev;
	}

	worstWeight = weight;
	for (unsigned int i = 0; i < nodes.size(); ++i) {
		worstWeight += nodes[i].devWeight * nodes[i].dev;
	}

}
