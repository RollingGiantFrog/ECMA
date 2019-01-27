#include <cstdlib>
#include <cmath>
#include <ilcplex/ilocplex.h>  

#include "instance.h"
#include "path.h"
#include "preprocess_nodes.h"
                  
using namespace std;

ILOSTLBEGIN

/*void printPath(IloCplex& cplex, IloArray<IloBoolVarArray>& x, int initNode, int endNode) {
	int currentNode = initNode;
	int prevNode = -1;
	while (currentNode != endNode) {
		cout << currentNode + 1 << " -> ";
		for (int v = 0; v < n; ++v) {
			if (adj[currentNode][v] && cplex.getValue(x[currentNode][v]) == 1) {
				currentNode = v;
				break;
			}
		}
	}
	cout << endNode + 1 << endl;
}*/

int main(int argc, char* argv[]){

	IloEnv env;
	
	IloModel model(env);

	if (argc < 2) return -1;
	Instance instance(argv[1],0);
	if (argc >= 3 && argv[2][0] == 'p')
		instance = preprocessInstance(instance);
	cout << "After preprocessing : " << instance.n << endl;
	
	try {
		IloArray<IloBoolVarArray> x(env, instance.n);
		for (int i = 0; i < instance.n; ++i) {
			x[i] = IloBoolVarArray(env, instance.n);
			for (int j = 0; j < instance.n; ++j) {
				if (instance.adj[i][j])
					x[i][j] = IloBoolVar(env);
			}
		}
	
		IloBoolVarArray y(env, instance.n);
		for (int j = 0; j < instance.n; ++j) {
			y[j] = IloBoolVar(env);
		}
		
		IloArray<IloNumVarArray> z(env, instance.n);
		for (int i = 0; i < instance.n; ++i) {
			z[i] = IloNumVarArray(env, instance.n);
			for (int j = 0; j < instance.n; ++j) {
				if (instance.adj[i][j])
					z[i][j] = IloNumVar(env);
			}
		}
	
		IloNumVarArray zp(env, instance.n);
		for (int j = 0; j < instance.n; ++j) {
			zp[j] = IloNumVar(env);
		}
			
		IloNumVar t1(env);
		IloNumVar t2(env);
		
	
		IloExpr obj(env);
		for (int i = 0; i < instance.n; ++i)
			for (int j = 0; j < instance.n; ++j)
				if (instance.adj[i][j]) obj += x[i][j] * instance.d[i][j] + instance.D[i][j] * z[i][j];
		obj += instance.d1 * t1;
		model.add(IloMinimize(env, obj));
		
		IloExpr expr(env);
		for (int j = 0; j < instance.n; ++j)
			if (instance.adj[instance.s][j]) expr += x[instance.s][j];
		model.add(expr == 1);
		expr.end();
		
		expr = IloExpr(env);
		for (int i = 0; i < instance.n; ++i)
			if (instance.adj[i][instance.t]) expr += x[i][instance.t];
		model.add(expr == 1);
		expr.end();
		
		for (int v = 0; v < instance.n; ++v) {
			if (v != instance.s && v != instance.t) {
				expr = IloExpr(env);
				for (int i = 0; i < instance.n; ++i)
					if (instance.adj[i][v]) expr += x[i][v];
					
				for (int j = 0; j < instance.n; ++j)
					if (instance.adj[v][j]) expr -= x[v][j];
				
				model.add(expr == 0);
				expr.end();
			}
		}
			
		for (int v = 0; v < instance.n; ++v) {
			if (v != instance.t) {
				expr = IloExpr(env);
				for (int j = 0; j < instance.n; ++j)
					if (instance.adj[v][j]) expr += x[v][j];
				model.add(expr == y[v]);
				expr.end();
			}
		}
		
		expr = IloExpr(env);
		for (int i = 0; i < instance.n; ++i)
			if (instance.adj[i][instance.t]) expr += x[i][instance.t];
		model.add(expr == y[instance.t]);
		expr.end();
		
		expr = IloExpr(env);
		for (int v = 0; v < instance.n; ++v)
			expr += instance.p[v]*y[v] + 2.*zp[v];
		expr += t2*instance.d2;
		model.add(expr <= instance.S);
		expr.end();
		
		for (int i = 0; i < instance.n; ++i)
			for (int j = 0; j < instance.n; ++j)
				if (instance.adj[i][j]) 
					model.add(t1 + z[i][j] >= instance.d[i][j]*x[i][j]);
		
		for (int v = 0; v < instance.n; ++v)
			model.add(t2 + zp[v] >= instance.ph[v]*y[v]);
		
		expr = IloExpr(env);
		for (int j = 0; j < instance.n; ++j)
			if (instance.adj[instance.t][j]) expr += x[instance.t][j];
		model.add(expr == 0);
		expr.end();
		
		IloCplex cplex(model);
		//cplex.setOut(env.getNullStream());

		
		cplex.setParam(IloCplex::MIPDisplay, 2);

		cplex.solve();

		if (cplex.getStatus() == IloAlgorithm::Infeasible)
			cout << "No Solution" << endl;

		else {
			cout << "objective: " << cplex.getObjValue() << endl;
			
		    //printPath(cplex,x,instance.s,instance.t);
		    
			for (int i = 0; i < instance.n; ++i) {
				x[i].end();
				z[i].end();
			}
			x.end();
			z.end();
			y.end();
			zp.end();
			t1.end();
			t2.end();
			
		}

	} 
	catch (const IloException& e){
		cerr << e;
		throw;
	}

	model.end();
	env.end();

}
