#include <cstdlib>
#include <cmath>
#include <ilcplex/ilocplex.h> 
#include <vector>

#include "instance.h"
#include "path.h"
#include "preprocess_nodes.h"
using namespace std;

ILOSTLBEGIN

void getPath(IloArray<IloBoolArray>& x, Instance instance, vector<int>& nodes) {
	int currentNode = instance.s;
	while (currentNode != instance.t) {
		nodes.push_back(currentNode);
		for (int v = 0; v < instance.n; ++v) {
			if (instance.hasEdge(currentNode,v) && (x[currentNode][v]) == 1) {
				currentNode = v;
				break;
			}
		}
	}
	nodes.push_back(instance.t);
}



int main(int argc, char** argv){

	IloEnv env;
	
	IloModel model(env);

	if (argc < 2) return -1;
	Instance instance(argv[1],0);
	if (argc >= 3 && argv[2][0] == 'p')
		instance = preprocessInstance(instance);
	cout << "After preprocessing : " << instance.n << endl;
	
	IloArray<IloBoolVarArray> x(env, instance.n);
	for (int i = 0; i < instance.n; ++i) {
		x[i] = IloBoolVarArray(env, instance.n);
		for (int j = 0; j < instance.n; j++) {
			if (instance.adj[i][j])
				x[i][j] = IloBoolVar(env);
		}
	}

	IloNumVar z(env);
	IloBoolVarArray y(env,instance.n);
	for (int i = 0; i < instance.n; ++i){
		y[i] = IloBoolVar(env);
	}
	IloExpr obj(env);
	obj += z;
	model.add(IloMinimize(env, obj));

	IloExpr leave_s(env);		
	for (int j = 0; j < instance.n; ++j) {
		if (j != instance.s){
			if (instance.hasEdge(instance.s,j)) {
				leave_s += x[instance.s][j];
			}
		}
	}
	model.add(leave_s == 1);
	
	IloExpr reach_t(env);
	for (int i = 0; i < instance.n; ++i) {
		if (i != instance.t) {
			if (instance.hasEdge(i,instance.t)) {
				reach_t += x[i][instance.t];
			}
		}
	}
	model.add(reach_t == 1);

	for (int i = 0; i < instance.n; ++i) {
		if ((i != instance.s) && (i != instance.t)) {
			IloExpr expr(env);
			for (int j = 0; j< instance.n; ++j) {
				if (instance.hasEdge(i,j)) expr += x[i][j];
				if (instance.hasEdge(j,i)) expr -= x[j][i];
			}
			model.add(expr == 0);
			expr.end();
		}
	}
	
	for (int i = 0; i < instance.n; ++i) {
		if (i != instance.t) {
			IloExpr expr(env);
			expr -= y[i];
			for (int j = 0; j < instance.n; ++j)
				if (instance.hasEdge(i,j)) expr += x[i][j];
			model.add(expr == 0);
			expr.end();
		}
	}
	
	IloExpr expr(env);
	expr -= y[instance.t];
	for (int i = 0; i < instance.n; ++i)
		if (instance.hasEdge(i,instance.t)) expr += x[i][instance.t];
	model.add(expr == 0);
	expr.end();

	expr = IloExpr(env);
	for (int j = 0; j < instance.n; ++j)
		if (instance.hasEdge(instance.t,j)) expr += x[instance.t][j];
	model.add(expr == 0);
	expr.end();

	IloCplex cplex(model);
	bool cutting = true; 
	while (cutting){
		cutting = false;
		cplex.solve();

		vector<int> nodes;
		int currentNode = instance.s;
		while (currentNode != instance.t) {
			nodes.push_back(currentNode);
			for (int i = 0; i < instance.neighbors[currentNode].size(); ++i) {
				if (cplex.getValue(x[currentNode][instance.neighbors[currentNode][i]]) > 0.99) {
					currentNode = instance.neighbors[currentNode][i];
					break;
				}
			}
		}
		nodes.push_back(instance.t);
		
		Path path(instance,nodes);

		if (cplex.getValue(z) < path.worstDist){
			cutting = true; 
			IloExpr cut1 = -z;
			for (int i = 0; i < instance.n; ++i) {
				for (int j = 0; j < instance.n; ++j) {
					if (instance.hasEdge(i,j)){
						cut1 +=  instance.d[i][j]*x[i][j];
					}
				}
			}
			for (unsigned int i = 0; i < path.edges.size(); ++i) {
				if (path.edges[i].dev > 0) cut1 += path.edges[i].dist*x[path.edges[i].node1][path.edges[i].node2]*path.edges[i].dev;
				else break;
			}
			model.add(cut1 <= 0);
		}

		if (path.worstWeight > instance.S){
			cutting = true;
			IloExpr cut2(env);
			for (int i = 0; i < instance.n; ++i) {
				cut2 += instance.p[i]*y[i];
			}
			for (unsigned int i = 0; i < path.nodes.size(); ++i) {
				if (path.nodes[i].dev > 0) cut2 += path.nodes[i].devWeight*y[path.nodes[i].node]*path.nodes[i].dev;
				else break;
			}
			model.add(cut2 <= instance.S);
		}
	}
	cout << "objective: " << cplex.getObjValue() << endl;
}