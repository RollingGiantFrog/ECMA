#include "instance.h"
#include "node.h"
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

Instance::Instance(char filename[], int verbosity) {
  char readChar;
  int readInt;
  int v1;
  int v2;
  int dist;
  float dev;

  ifstream file(filename);
  if(file) {
    file >> readChar >> readChar >> n;
    if (verbosity >= 1) cout << "n = " << n << endl;

    file >> readChar >> readChar >> s;
    s = s - 1;
    if (verbosity >= 1) cout << "s = " << s << endl;

    file >> readChar >> readChar >> t;
    t = t - 1;
    if (verbosity  >= 1) cout << "t = " << t << endl;

    file >> readChar >> readChar >> S;
    if (verbosity >= 1) cout << "S = " << S << endl;

    file >> readChar >> readInt >> readChar >> d1;
    if (verbosity >= 1) cout << "d1 = " << d1 << endl;

    file >> readChar >> readInt >> readChar >> d2;
    if (verbosity >= 1) cout << "d2 = " << d2 << endl;

	file >> readChar >> readChar >> readChar;
	for (int i = 0; i < n; ++i) {
		file >> readInt >> readChar;
		p.push_back(readInt);
	}
	if (verbosity >= 2) {
		cout << "p = [";
		for (int i = 0; i < n; ++i)
			cout << p[i] << " ";
		cout << "]" << endl;
	}

	file >> readChar >> readChar >> readChar >> readChar;
	for (int i = 0; i < n; ++i) {
		file >> readInt >> readChar;
		ph.push_back(readInt);
	}
	if (verbosity >= 2) {
		cout << "ph = [";
		for (int i = 0; i < n; ++i)
			cout << ph[i] << " ";
		cout << "]" << endl;
	}

    m = 0;
	for (int i = 0; i < n; ++i) {
        initialNodes.push_back(i);
		adj.push_back(vector<bool>(n,false));
		d.push_back(vector<int>(n,0));
		D.push_back(vector<float>(n,0.));
		neighbors.push_back(vector<Node>());
		predecessors.push_back(vector<Node>());
	}

	file >> readChar >> readChar >> readChar >> readChar;

	while (file.peek() != ']') {
		file >> readChar >> v1 >> v2 >> dist >> dev;
		d[v1-1][v2-1] = dist;
		D[v1-1][v2-1] = dev;

		if (dist != 0) {
            adj[v1-1][v2-1] = true;
            m = m + 1;
            neighbors[v1-1].push_back(v2-1);
            predecessors[v2-1].push_back(v1-1);
		}
	}

    file.close();
  }
}

bool Instance::hasEdge(int node1, int node2) const {
	return adj[node1][node2];
}

int Instance::nodeWeight(int node) const {
	return p[node];
}

int Instance::edgeDist(int node1, int node2) const {
	return d[node1][node2];
}

Instance Instance::restrict(std::set<Node> removedNodes, std::set<Edge> removedEdges) {
    Instance instance;
    instance.n = 0;
    instance.S = S;
    instance.d1 = d1;
    instance.d2 = d2;

    std::vector<Node> instanceToSelf;
    std::vector<Node> selfToInstance(n,-1);

    for (unsigned int i = 0; i < n; ++i) {
        if (removedNodes.find(i) == removedNodes.end()) {
            selfToInstance[i] = instance.n;
            instanceToSelf.push_back(i);

            instance.n = instance.n + 1;
            instance.p.push_back(p[i]);
            instance.ph.push_back(ph[i]);
            instance.initialNodes.push_back(initialNodes[i]);
        }
    }

    instance.s = selfToInstance[s];
    instance.t = selfToInstance[t];

    instance.m = 0;
    for (int i = 0; i < instance.n; ++i) {
		instance.adj.push_back(vector<bool>(instance.n,false));
		instance.d.push_back(vector<int>(instance.n,0));
		instance.D.push_back(vector<float>(instance.n,0.));
		instance.neighbors.push_back(vector<Node>());
		instance.predecessors.push_back(vector<Node>());
	}

    for (int i = 0; i < instance.n; ++i) {
        for (unsigned int j = 0; j < neighbors[instanceToSelf[i]].size(); ++j) {
            Node v = neighbors[instanceToSelf[i]][j];
            if (removedNodes.find(v) == removedNodes.end() && removedEdges.find(std::make_pair(instanceToSelf[i],v)) == removedEdges.end()) {
                instance.neighbors[i].push_back(selfToInstance[v]);
                instance.predecessors[selfToInstance[v]].push_back(i);
                instance.adj[i][selfToInstance[v]] = true;
                instance.m = instance.m + 1;
                instance.d[i][selfToInstance[v]] = d[instanceToSelf[i]][v];
                instance.D[i][selfToInstance[v]] = D[instanceToSelf[i]][v];
            }
        }
    }

    return instance;
}
