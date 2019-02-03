#include <cstdlib>
#include <cmath>
#include <ilcplex/ilocplex.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "instance.h"
#include "path.h"
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

// Fonction qui utilise cplex pour résoudre le PLNE associé à la dualisation du problème robuste
InfoSolution dualization(const Instance& instance,  int TimeLim){
	double start = get_time();
	IloEnv env;
	IloModel model(env);
	try {
		// définition des variables du modèle
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
		
		// définition de l'objectif
		IloExpr obj(env);
		for (int i = 0; i < instance.n; ++i)
			for (int j = 0; j < instance.n; ++j)
				if (instance.adj[i][j]) obj += x[i][j] * instance.d[i][j] + instance.D[i][j] * z[i][j];
		obj += instance.d1 * t1;
		model.add(IloMinimize(env, obj));
		obj.end();
		
		// définition des contraintes du modèle
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
		
		// resolution avec cplex
		IloCplex cplex(model);
		cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::TiLim, TimeLim);
		cplex.solve();

		IloAlgorithm::Status st = cplex.getStatus();
		double v;
		double bound;
		vector<int> nodes;

		if (st != IloAlgorithm::Infeasible){
			v = cplex.getObjValue();
			bound = cplex.getBestObjValue();
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
		}
		nodes.push_back(instance.t);
		double end = get_time();
		double tWithoutClosing = end - start;
		// suppresion des variables
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
			return InfoSolution({-1.,0.,0,false, 0,"dualization",nodes});
		}
		else
			return InfoSolution({v,bound, end - start, 0.9999*v <= bound,tWithoutClosing,"dualization",nodes}); 
	} 
	catch (const IloException& e){
		cerr << e;
		model.end();
		env.end();
		return InfoSolution({-2.,0.,0,false, 0,"dualization"});
	}

}

// dualisation avec un chemin donné pour initialiser la résolution
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
		cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::TiLim, TimeLim);
		// Ajout de la solution pour initialiser cplex
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
		model.end();
		env.end();
		return InfoSolution({-2.,0.,0,false, 0,"dualization"});
	}
}

// methode de plans coupants
InfoSolution cuttingplanes(const Instance& instance,  int TimeLim){
	double start = get_time();
	IloEnv env;
	IloModel model(env);
	IloArray<IloBoolVarArray> x(env, instance.n);
	vector<Node> feasibleSolution;
	// meilleure valeur pour une solution réalisable du problème initial
	double feasibleDist = 1000000000000.;

	// definition des variables du probleme maître
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
	
	// définition des contraintes du problème maître
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
	cplex.setOut(env.getNullStream());
	cplex.setParam(IloCplex::Param::Threads,1);
	cplex.solve();
	bool cutting = true;
	double end = get_time();
	double t = end - start;
	// tant que l'algorithme effectue des coupes et qu'on ne depasse pas le temps fixé
	while (cutting && (t < double(TimeLim))){

		// ce block effectue une résolution de sac à dos continu pour les sous-problèmes
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
		// on a ajouté 0.9999 pour assurer la terminaison de l'algorithme
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
		// dans le cas où une solution verifie la contrainte de poids, on teste si c'est la meilleure valeur obtenue jusqu'à présent
		// si c'est le cas, alors c'est la meilleure solution réalisable qu'on ait obtenu jusqu'ici donc on l'enregistre
		else{
			if (path.worstDist < feasibleDist){
				feasibleDist = path.worstDist;
				feasibleSolution = nodes;
			}
		}
		cplex.solve();
		end = get_time();
		t = end - start;
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
		return InfoSolution({feasibleDist, feasibleDist, t, true, t1,"cuttingplanes", feasibleSolution});
	else 
		return InfoSolution({feasibleDist, -1., t, false, t1,"cuttingplanes", feasibleSolution});
}

// il s'agit du même code que précédemment mais partant d'une solution initiale réalisable dans stockée dans path
InfoSolution cuttingplanes(const Instance& instance,  int TimeLim, const Path& path){
	double start = get_time();
	IloEnv env;
	IloModel model(env);
	IloNumVar z(env);
	IloBoolVarArray y(env,instance.n);
	vector<Node> feasibleSolution;
	double feasibleDist = 1000000000000.;
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
	// comme la solution initiale donnée est supposée réalisable, elle va donc induire la coupe suivante sur les deviations de ses distances
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
	model.add(cut2 <= instance.S);
	cut2.end();
	//initialisation du modèle avec la solution donnée en input
	IloNumVarArray startVar(env);
 	IloNumArray startVal(env);
 	bool nodesInPath[instance.n] = {false};
 	bool edgesInPath[instance.n][instance.n] = {false};
 	for (int i = 0; i < path.path.size(); ++i)
 		nodesInPath[path.path[i]] = true;

 	for (int i = 0; i < path.path.size()-1; ++i)
 		edgesInPath[path.path[i]][path.path[i+1]] = true;

  
  for (int i = 0; i < instance.n; ++i){
  	if (nodesInPath[i]) {
  		startVar.add(y[i]);
  		startVal.add(true);
  	}
  	else {
  		startVar.add(y[i]);
  		startVal.add(false);
  	}
  	for (int j = 0; j < instance.n; ++j){
			if (edgesInPath[i][j]) {
  			startVar.add(x[i][j]);
  			startVal.add(true);
  		}
  		else {
  			if (instance.adj[i][j]){
  				startVar.add(x[i][j]);
  				startVal.add(false);
  			}
  		}
	  }
	}
  startVar.add(z);
  startVal.add(path.worstDist);
	cplex.addMIPStart(startVar, startVal);
  startVal.end();
  startVar.end();
  cplex.solve();

	double end = get_time();
	bool cutting = true;
 	double t = end - start;		
	while (cutting && (t < double(TimeLim))){
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
		// on a ajouté 0.9999 pour assurer la terminaison de l'algorithme
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

		else{
			if (path.worstDist < feasibleDist)
				feasibleSolution = nodes;
		}
		cplex.solve();
		end = get_time();
		t = end - start;
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
		return InfoSolution({feasibleDist, feasibleDist, t, true, t1,"cuttingplanes", feasibleSolution});
	else 
		return InfoSolution({feasibleDist, -1., t, false, t1,"cuttingplanes", feasibleSolution});
}


// definition d'un schema de coupes pour un algorithme de branch and cut introduisant le contraintes
// on résout les deux sous-problèmes comme des problèmes de sac-à-dos continus
// on passe en argument un pointeur vers la meilleur solution réalisable du problème aître qui
// est mise à jour si on en trouve une meilleure lors de l'appel de lazyCallbackCplexOpt
ILOLAZYCONSTRAINTCALLBACK7(lazyCallbackCplexOpt,
								 IloCplex, cplex,
								 Instance, instance,
			                     IloArray<IloBoolVarArray>, x,
			                     IloIntVarArray, y,
			                     IloNumVar, z,
			                     vector<Node>*, bestFeasibleSol,
			                     double*, bestFeasibleVal)
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
	else{
		if(path.worstDist < *bestFeasibleVal){
			*bestFeasibleVal = path.worstDist;
			*bestFeasibleSol = nodes;
		}
	}
}

InfoSolution branchandcut(const Instance& instance,  int TimeLim){
	double start = get_time();
	IloEnv env;
	IloModel model(env);
	// définition des variables du problème maître
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
	// definition de l'objectif du problème maître
	IloExpr obj(env);
	obj += z;
	model.add(IloMinimize(env, obj));
	//définition des contraintes du problème maître
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

	// coupes initiales
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
	double bestFeasibleVal = 1000000000.;
	vector<Node> bestFeasibleSol;
	// utilisation des coupes 
	cplex.use(lazyCallbackCplexOpt(env,cplex,instance,x,y,z,&bestFeasibleSol,&bestFeasibleVal));
	cplex.setParam(IloCplex::CutsFactor, 1.0);
	cplex.setParam(IloCplex::EachCutLim, 0);
	cplex.setParam(IloCplex::PreInd, 0);
	cplex.setParam(IloCplex::TiLim, TimeLim);
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
	return InfoSolution({v,b, t, b > 0.9999*v, t1, "branchandcut", bestFeasibleSol});
}

// meme programme que ci-dessus mais avec une solution réalisable donnée en input dans la variable path
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
	//cplex.use(lazyCallbackCplexOpt(env,cplex,instance,x,y,z));
	cplex.setParam(IloCplex::CutsFactor, 1.0);
	cplex.setParam(IloCplex::EachCutLim, 0);
	cplex.setParam(IloCplex::PreInd, 0);
	cplex.setParam(IloCplex::TiLim, TimeLim);
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