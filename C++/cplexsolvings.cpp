#include <cstdlib>
#include <cmath>
#include <ilcplex/ilocplex.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "instance.h"
#include "path.h"
#include "preprocess_nodes.h"
#include "infosolution.h"
#include "shortest_capacited_path.h"
                  
using namespace std;
ILOSTLBEGIN


double get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}


InfoSolution dualization(const Instance& instance,  int TimeLim){
	double start = get_time();
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
		obj.end();
		
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
		cplex.setParam(IloCplex::Param::Threads,1);
		//cplex.setParam(IloCplex::PreInd, 0);
		
		//cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::MIPDisplay, 4);
		cplex.setParam(IloCplex::TiLim, TimeLim);
		cplex.solve();

		IloAlgorithm::Status st = cplex.getStatus();
		double v;
		double bound;
		if (st != IloAlgorithm::Infeasible){
			v = cplex.getObjValue();
			bound = cplex.getBestObjValue();
		}
		double end = get_time();
		double tWithoutClosing = end - start;
		for (int i = 0; i < instance.n; ++i) {
			for (int j=0; j < instance.n; ++j){
				if (instance.adj[i][j]){
					x[i][j].end();
					z[i][j].end();
				}
			}
			y[i].end();
			zp[i].end();
			x[i].end();
			z[i].end();
		}
		x.end();
		z.end();
		y.end();
		zp.end();
		t1.end();
		t2.end();
		end = get_time();
		model.end();
		env.end();
		if (st == IloAlgorithm::Infeasible){
			cout << "No Solution" << endl;
			return InfoSolution({-1.,0.,0,false, 0,"dualization"});
		}
		else {
			return InfoSolution({v,bound, end - start, 0.9999*v <= bound,tWithoutClosing,"dualization"}); 
		}
	} 
	catch (const IloException& e){
		cerr << e;
		return InfoSolution({-2.,0.,0,false, 0,"dualization"});
	}

}

// dualization using a given path to initiate the MIP
InfoSolution dualization(const Instance& instance,  int TimeLim, const Path& path){
	double start = get_time();
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
		for (int j = 0; j < instance.n; ++j)
			y[j] = IloBoolVar(env);
		
		IloArray<IloNumVarArray> z(env, instance.n);
		for (int i = 0; i < instance.n; ++i) {
			z[i] = IloNumVarArray(env, instance.n);
			for (int j = 0; j < instance.n; ++j)
				if (instance.adj[i][j])
					z[i][j] = IloNumVar(env);
		}
	
		IloNumVarArray zp(env, instance.n);
		for (int j = 0; j < instance.n; ++j)
			zp[j] = IloNumVar(env);
			
		IloNumVar t1(env);
		IloNumVar t2(env);
		
	
		IloExpr obj(env);
		for (int i = 0; i < instance.n; ++i)
			for (int j = 0; j < instance.n; ++j)
				if (instance.adj[i][j]) obj += x[i][j] * instance.d[i][j] + instance.D[i][j] * z[i][j];
		obj += instance.d1 * t1;
		model.add(IloMinimize(env, obj));
		obj.end();

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
		//cplex.setParam(IloCplex::Param::Threads,1);
		//cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::MIPDisplay, 4);
		cplex.setParam(IloCplex::TiLim, TimeLim);


		IloNumVarArray startVar(env);
   	IloNumArray startVal(env);
   	IloArray<IloBoolArray> x_start(env, instance.n);
    for (int i = 0; i < path.length - 2; ++i){
    	startVar.add(x[path.path[i]][path.path[i+1]]);
    	startVal.add(true);
    }
		cplex.addMIPStart(startVar, startVal);
    startVal.end();
    startVar.end();

		cplex.solve();

		IloAlgorithm::Status st = cplex.getStatus();
		double v;
		double bound;
		if (st != IloAlgorithm::Infeasible){
			v = cplex.getObjValue();
			bound = cplex.getBestObjValue();
		}
		double end = get_time();
		double tWithoutClosing = end - start;
		for (int i = 0; i < instance.n; ++i) {
			for (int j=0; j < instance.n; ++j){
				if (instance.adj[i][j]){
					x[i][j].end();
					z[i][j].end();
				}
			}
			y[i].end();
			zp[i].end();
			x[i].end();
			z[i].end();
		}
		x.end();
		z.end();
		y.end();
		zp.end();
		t1.end();
		t2.end();
		end = get_time();
		model.end();
		env.end();
		if (st == IloAlgorithm::Infeasible){
			cout << "No Solution" << endl;
			return InfoSolution({-1.,0.,0,false, 0,"dualization"});
		}
		else
			return InfoSolution({v,bound, end - start, 0.9999*v <= bound,tWithoutClosing,"dualization"}); 
	} 
	catch (const IloException& e){
		cerr << e;
		return InfoSolution({-2.,0.,0,false, 0,"dualization"});
	}
}


InfoSolution cuttingplanes(const Instance& instance,  int TimeLim){
	double start = get_time();
	IloEnv env;
	IloModel model(env);
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
	obj.end();

	IloExpr leave_s(env);		
	for (int j = 0; j < instance.n; ++j) {
		if (j != instance.s){
			if (instance.hasEdge(instance.s,j)) {
				leave_s += x[instance.s][j];
			}
		}
	}
	model.add(leave_s == 1);
	leave_s.end();
	IloExpr reach_t(env);
	for (int i = 0; i < instance.n; ++i) {
		if (i != instance.t) {
			if (instance.hasEdge(i,instance.t)) {
				reach_t += x[i][instance.t];
			}
		}
	}
	model.add(reach_t == 1);
	reach_t.end();
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
	cplex.setParam(IloCplex::Param::Threads,1);
	IloExpr cut1 = -z;
	for (int i = 0; i < instance.n; ++i) {
		for (int j = 0; j < instance.n; ++j) {
			if (instance.hasEdge(i,j)){
				cut1 +=  instance.d[i][j]*x[i][j];
			}
		}
	}
	model.add(cut1 <= 0);
	cut1.end();
	IloExpr cut2(env);
	for (int i = 0; i < instance.n; ++i) {
		cut2 += instance.p[i]*y[i];
	}
	model.add(cut2 <= instance.S);
	cut2.end();
	//cplex.setOut(env.getNullStream());
	cplex.setParam(IloCplex::Param::Threads,1);
	cplex.solve();
	bool cutting = true;
	double end = get_time();
	double t = end - start;
	while (cutting and t < TimeLim){
		cutting = false;

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

		if (cplex.getValue(z) < 0.9999*path.worstDist){
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

		if (path.worstWeight*0.9999 > instance.S){
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
			cut2.end();
		}
		cplex.solve();
		double end = get_time();
		double t = end - start;
		//cout << "time :" << t << endl;
	}
	double t1 = t;
	double v = cplex.getObjValue();
	for(int i = 0; i < instance.n; ++i)
		x[i].end();
	x.end();
	z.end();
	y.end();
	model.end();
	env.end();
	end = get_time();
	t = end - start;
	

	if (!cutting)
		return InfoSolution({v, v, t, true, t1,"cuttingplanes"});
	else 
		return InfoSolution({-1., v, t, false, t1,"cuttingplanes"});
}

InfoSolution cuttingplanes(const Instance& instance,  int TimeLim, const Path& path){
	double start = get_time();
	IloEnv env;
	IloModel model(env);
	IloNumVar z(env);
	IloBoolVarArray y(env,instance.n);
	IloArray<IloBoolVarArray> x(env, instance.n);
	for (int i = 0; i < instance.n; ++i) {
		x[i] = IloBoolVarArray(env, instance.n);
		for (int j = 0; j < instance.n; j++) {
			if (instance.adj[i][j])
				x[i][j] = IloBoolVar(env);
		}
	}
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
	leave_s.end();

	IloExpr reach_t(env);
	for (int i = 0; i < instance.n; ++i) {
		if (i != instance.t) {
			if (instance.hasEdge(i,instance.t)) {
				reach_t += x[i][instance.t];
			}
		}
	}
	model.add(reach_t == 1);
	reach_t.end();

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
	cplex.setParam(IloCplex::Param::Threads,1);
	cplex.setOut(env.getNullStream());
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
	cut1.end();
	IloExpr cut2(env);
	for (int i = 0; i < instance.n; ++i) {
		cut2 += instance.p[i]*y[i];
	}
	for (unsigned int i = 0; i < path.nodes.size(); ++i) {
		if (path.nodes[i].dev > 0) cut2 += path.nodes[i].devWeight*y[path.nodes[i].node]*path.nodes[i].dev;
		else break;
	}
	model.add(cut2 <= instance.S);
	cut2.end();
	cplex.solve();
	double end = get_time();
	bool cutting = true;
 	double t = end - start;		
 	while (cutting and t < TimeLim){
		cutting = false;


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
		
		Path path2(instance,nodes);
		if (cplex.getValue(z) < 0.9999*path2.worstDist){
			cutting = true; 
			IloExpr cut1 = -z;
			for (int i = 0; i < instance.n; ++i) {
				for (int j = 0; j < instance.n; ++j) {
					if (instance.hasEdge(i,j)){
						cut1 +=  instance.d[i][j]*x[i][j];
					}
				}
			}
			for (unsigned int i = 0; i < path2.edges.size(); ++i) {
				if (path2.edges[i].dev > 0) cut1 += path2.edges[i].dist*x[path2.edges[i].node1][path2.edges[i].node2]*path2.edges[i].dev;
				else break;
			}
			model.add(cut1 <= 0);
			cut1.end();
		}

		if (path2.worstWeight*0.9999 > instance.S){
			cutting = true;
			IloExpr cut2(env);
			for (int i = 0; i < instance.n; ++i) {
				cut2 += instance.p[i]*y[i];
			}
			for (unsigned int i = 0; i < path2.nodes.size(); ++i) {
				if (path2.nodes[i].dev > 0) cut2 += path2.nodes[i].devWeight*y[path2.nodes[i].node]*path2.nodes[i].dev;
				else break;
			}
			model.add(cut2 <= instance.S);
			cut2.end();
		}
		cplex.solve();
		double end = get_time();
		double t = end - start;

	}
	double v = cplex.getObjValue();
	double t1 = t;
	z.end();
	y.end();
	for(int i = 0; i < instance.n; ++i)
		x[i].end();
	x.end();
	model.end();
	env.end();
	end = get_time();
	t = end - start;
	if (!cutting)
		return InfoSolution({v,v, t, true, t1, "cuttingplanes"});
	else 
		return InfoSolution({-1., v, t, false, t1, "cuttingplanes"});
}



ILOLAZYCONSTRAINTCALLBACK5(lazyCallbackCplexOpt,
								 IloCplex, cplex,
								 Instance, instance,
			                     IloArray<IloBoolVarArray>, x,
			                     IloIntVarArray, y,
			                     IloNumVar, z)
{
	IloEnv masterEnv = getEnv();
	
	vector<int> nodes;
	int currentNode = instance.s;
	while (currentNode != instance.t) {
		nodes.push_back(currentNode);
		for (int i = 0; i < instance.neighbors[currentNode].size(); ++i) {
			if (getValue(x[currentNode][instance.neighbors[currentNode][i]]) > 0.99) {
				currentNode = instance.neighbors[currentNode][i];
				break;
			}
		}
	}
	nodes.push_back(instance.t);
	//getPath(cplex, x, instance, nodes);
	Path path(instance,nodes);
	
	if (getValue(z) < 0.9999*path.worstDist){
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
		add(cut1 <= 0);
		cut1.end();
	}

	if (path.worstWeight*0.9999 > instance.S){
		IloExpr cut2(masterEnv);
		for (int i = 0; i < instance.n; ++i) {
			cut2 += instance.p[i]*y[i];
		}
		for (unsigned int i = 0; i < path.nodes.size(); ++i) {
			if (path.nodes[i].dev > 0) cut2 += path.nodes[i].devWeight*y[path.nodes[i].node]*path.nodes[i].dev;
			else break;
		}
		add(cut2 <= instance.S);
		cut2.end();
	}
}

InfoSolution branchandcut(const Instance& instance,  int TimeLim){
	double start = get_time();
	IloEnv env;
	IloModel model(env);
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
	leave_s.end();
	
	IloExpr reach_t(env);
	for (int i = 0; i < instance.n; ++i) {
		if (i != instance.t) {
			if (instance.hasEdge(i,instance.t)) {
				reach_t += x[i][instance.t];
			}
		}
	}
	model.add(reach_t == 1);
	reach_t.end();

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

	IloExpr cut1 = -z;
	for (int i = 0; i < instance.n; ++i) {
		for (int j = 0; j < instance.n; ++j) {
			if (instance.hasEdge(i,j)){
				cut1 +=  instance.d[i][j]*x[i][j];
			}
		}
	}
	model.add(cut1 <= 0);
	cut1.end();
	IloExpr cut2(env);
	for (int i = 0; i < instance.n; ++i) {
		cut2 += instance.p[i]*y[i];
	}
	model.add(cut2 <= instance.S);
	cut2.end();

	IloCplex cplex(model);
	cplex.setParam(IloCplex::Param::Threads,1);
	cplex.setOut(env.getNullStream());
	cplex.use(lazyCallbackCplexOpt(env,cplex,instance,x,y,z));
	cplex.setParam(IloCplex::CutsFactor, 1.0);
	cplex.setParam(IloCplex::EachCutLim, 0);
	cplex.setParam(IloCplex::PreInd, 0);
	cplex.setParam(IloCplex::TiLim, TimeLim);
	cplex.setParam(IloCplex::MIPDisplay, 0);
	cplex.solve();

	
	double v = cplex.getObjValue();
	double b = cplex.getBestObjValue();
	double end = get_time();
	double t1 = end - start;
	for(int i = 0; i < instance.n; ++i)
		x[i].end();
	x.end();
	z.end();
	y.end();
	model.end();
	env.end();
	end = get_time();
	double t = end - start;
	return InfoSolution({v,b, t, b > 0.9999*v, t1, "branchandcut"});
}

InfoSolution branchandcut(const Instance& instance,  int TimeLim, const Path& path){
	double start = get_time();
	IloEnv env;
	IloModel model(env);
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
	leave_s.end();
	
	IloExpr reach_t(env);
	for (int i = 0; i < instance.n; ++i) {
		if (i != instance.t) {
			if (instance.hasEdge(i,instance.t)) {
				reach_t += x[i][instance.t];
			}
		}
	}
	model.add(reach_t == 1);
	reach_t.end();
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
	cut1.end();
	IloExpr cut2(env);
	for (int i = 0; i < instance.n; ++i) {
		cut2 += instance.p[i]*y[i];
	}
	for (unsigned int i = 0; i < path.nodes.size(); ++i) {
		if (path.nodes[i].dev > 0) cut2 += path.nodes[i].devWeight*y[path.nodes[i].node]*path.nodes[i].dev;
		else break;
	}
	model.add(cut2 <= instance.S);
	cut2.end();

	IloCplex cplex(model);
	cplex.setOut(env.getNullStream());
	cplex.setParam(IloCplex::Param::Threads, 1);
	cplex.use(lazyCallbackCplexOpt(env,cplex,instance,x,y,z));
	cplex.setParam(IloCplex::CutsFactor, 1.0);
	cplex.setParam(IloCplex::EachCutLim, 0);
	cplex.setParam(IloCplex::PreInd, 0);
	cplex.setParam(IloCplex::TiLim, TimeLim);
	cplex.setParam(IloCplex::MIPDisplay, 0);
	cplex.solve();

	double v = cplex.getObjValue();
	double b = cplex.getBestObjValue();
	double end = get_time();
	double t1 = end - start;
	for(int i = 0; i < instance.n; ++i)
		x[i].end();
	x.end();
	z.end();
	y.end();
	model.end();
	env.end();
	end = get_time();
	double t = end - start;
	return InfoSolution({v,b, t, b > 0.9999*v, t1,"branchandcut"});
}



/*int main(int argc, char** argv){
	if (argc < 2) return -1;
	Instance instance(argv[1],0);
	if (argc >= 3 && argv[2][0] == 'p'){
		instance = preprocessInstance(instance);
		cout << "After preprocessing : " << instance.n << endl;
	}
	//InfoSolution inf = dualization(instance,60);
	InfoSolution inf = cuttingplanes(instance, 10);
	cout << "cuttingplanes done" << endl;
	cout << inf.solution << endl;
	cout << "time : " << inf.t << endl;
	inf = branchandcut(instance, 10);
	cout << "branchandcut done" << endl;
	cout << inf.solution << endl;
	cout << "time : " << inf.t << endl;
	inf = dualization(instance, 10);
	cout << "dualization done" << endl;
	cout << inf.solution << endl;
	cout << "time : " << inf.t << endl;

	SemiWorstCaseNodeMetric SWCNM(instance);
  SemiWorstCaseEdgeMetric SWCEM(instance);
	ShortestCapacitedPath<SemiWorstCaseNodeMetric, SemiWorstCaseEdgeMetric> SCP(instance, instance.s, instance.t, SWCNM, SWCEM);
	Path path = SCP.extractPathNodes(instance.s, instance.t, -1); 
	//InfoSolution inf2 = dualization(instance, 60, path);
	cout << "cuttingplanes with heuristic value done" << endl;
	InfoSolution inf2 = cuttingplanes(instance, 10, path);
	cout << inf2.solution << endl;
	cout << "time : " << inf2.t << endl;
}*/