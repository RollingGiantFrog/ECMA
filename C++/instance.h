#ifndef INSTANCE_H
#define INSTANCE_H

#include <vector>
#include <set>
#include "node.h"

using namespace std;

class Instance {
public:
    Instance() {};
	Instance(char filename[], int verbosity);
	bool hasEdge(Node node1, Node node2) const;
	int nodeWeight(Node node) const;
	int edgeDist(Node node1, Node node2) const;
	Instance restrict(std::set<Node> removedNodes, std::set<Edge> removedEdges);

	int n;
	int m;

	vector<Node> initialNodes;

	Node s;
	Node t;

	int d1;
	int d2;

	int S;
	vector<Node> p;
	vector<Node> ph;

	vector< vector<bool> > adj;
	vector< vector<int> > d;
	vector< vector<float> > D;

	vector< vector<Node> > neighbors;
	vector< vector<Node> > predecessors;
};


#endif
