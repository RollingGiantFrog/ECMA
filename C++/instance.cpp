#include "instance.h"
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
	
	for (int i = 0; i < n; ++i) {
		adj.push_back(vector<bool>(n,false));
		d.push_back(vector<int>(n,0));
		D.push_back(vector<float>(n,0.));
		neighbors.push_back(vector<int>());
	}
	
	file >> readChar >> readChar >> readChar >> readChar;
	
	while (file.peek() != ']') {
		file >> readChar >> v1 >> v2 >> dist >> dev;
		d[v1-1][v2-1] = dist;
		D[v1-1][v2-1] = dev;
		
		if (dist != 0) adj[v1-1][v2-1] = true;
		if (dist != 0) neighbors[v1-1].push_back(v2-1);
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
