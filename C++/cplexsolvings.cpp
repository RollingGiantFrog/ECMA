#include <cstdlib>
#include <cmath>
#include <ilcplex/ilocplex.h>  

#include "instance.h"
#include "path.h"
#include "preprocess_nodes.h"
#include "infosolution.h"
#include "shortest_capacited_path.h"
                  
using namespace std;

ILOSTLBEGIN

InfoSolution dualization(const Instance& instance,  int TimeLim){
	clock_t start = clock();
	IloEnv env;
	IloModel model(env);
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
		cplex.setParam(IloCplex::MIPDisplay, 2);
		cplex.setParam(IloCplex::TiLim, TimeLim);
		cplex.solve();

		if (cplex.getStatus() == IloAlgorithm::Infeasible){
			cout << "No Solution" << endl;
			return InfoSolution({-1.,0.,0,false});
		}
		else {
			float v = cplex.getObjValue();
			float bound = cplex.getBestObjValue(); 
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
			clock_t end = clock();
			//cplex.getBestObjValue()
			//(cplex.getObjValue()*(0.9999) <= cplex.getBestObjValue())
			return InfoSolution({v,bound, float (end -start)/CLOCKS_PER_SEC * 1000., 0.9999*v <= bound }); 
		}
	} 
	catch (const IloException& e){
		cerr << e;
		return InfoSolution({-2.,0.,0,false});
	}
	model.end();
	env.end();
}

// dualization using a given path to initiate the MIP
InfoSolution dualization(const Instance& instance,  int TimeLim, Path path){
	clock_t start = clock();
	IloEnv env;
	IloModel model(env);


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
		

    /*
    for (int i = 0; i < instance.n, ++i){
    	starVae.add(y[])*/


		IloCplex cplex(model);
		cplex.setParam(IloCplex::MIPDisplay, 2);
		cplex.setParam(IloCplex::TiLim, TimeLim);


		IloNumVarArray startVar(env);
   	IloNumArray startVal(env);
   	//IloArray<IloBoolArray> x_start(env, instance.n);
   	// IloBoolArray y_start(env, instance.n)
    /*for (int i = 0; i < instance.n; ++i){
    	x_start[i] = IloBoolArray(env, instance.n);
    	//y_start[i] = 0;
    	for (int j = 0; j < instance.n; ++j)
        x_start[i][j] = 0;
    }*/
    for (int i = 0; i < path.length - 2; ++i){
    	startVar.add(x[path.path[i]][path.path[i+1]]);
    	startVal.add(true);
    	//y_start[path.nodes[i].node] = 1;
    }
    /*
    for (int i = 0; i < instance.n; ++i){
    	for (int j = 0; j < instance.n; ++j){
    		startVar.add(x[i][j]);
        startVal.add(x_start[i][j]);
    	}
    }*/
		cplex.addMIPStart(startVar, startVal);
    startVal.end();
    startVar.end();

		cplex.solve();

		if (cplex.getStatus() == IloAlgorithm::Infeasible){
			cout << "No Solution" << endl;
			return InfoSolution({-1.,0.,0,false});
		}
		else {
			float v = cplex.getObjValue();
			float bound = cplex.getBestObjValue(); 
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
			clock_t end = clock();
			//cplex.getBestObjValue()
			//(cplex.getObjValue()*(0.9999) <= cplex.getBestObjValue())
			return InfoSolution({v,bound, float (end -start)/CLOCKS_PER_SEC * 1000., 0.9999*v <= bound }); 		
		}
	} 
	catch (const IloException& e){
		cerr << e;
		return InfoSolution({-2.,0.,0,false});
	}
	model.end();
	env.end();
}

int main(int argc, char** argv){
	if (argc < 2) return -1;
	Instance instance(argv[1],0);
	if (argc >= 3 && argv[2][0] == 'p')
		instance = preprocessInstance(instance);
	cout << "After preprocessing : " << instance.n << endl;
	InfoSolution inf = dualization(instance,60);
	cout << "method 1 done" << endl;
	SemiWorstCaseNodeMetric SWCNM(instance);
  SemiWorstCaseEdgeMetric SWCEM(instance);
	ShortestCapacitedPath<SemiWorstCaseNodeMetric, SemiWorstCaseEdgeMetric> SCP(instance, instance.s, instance.t, SWCNM, SWCEM);
	Path path = SCP.extractPathNodes(instance.s, instance.t, -1); 
	InfoSolution inf2 = dualization(instance, 60, path);
}