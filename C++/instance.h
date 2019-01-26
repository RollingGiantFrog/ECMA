#ifndef INSTANCE_H
#define INSTANCE_H

#include <vector>

using namespace std;


class Instance {
public:
	Instance(char filename[], int verbosity);
	bool hasEdge(int node1, int node2) const;
	int nodeWeight(int node) const;
	int edgeDist(int node1, int node2) const;
	
	int n;
	
	int s;
	int t;
	
	int d1;
	int d2;
	
	int S;
	vector<int> p;
	vector<int> ph;
	
	vector< vector<bool> > adj;
	vector< vector<int> > d;
	vector< vector<float> > D;
	
	vector< vector<int> > neighbors;
};


#endif
