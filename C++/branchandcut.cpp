#include <cstdlib>
#include <cmath>
#include <ilcplex/ilocplex.h> 
#include <vector>

#include "instance.h"
#include "path.h"
                   
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

// Optimisation avec le solveur cplex
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
	
	if (getValue(z) < path.worstDist){
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
	}

	if (path.worstWeight > instance.S){
		IloExpr cut2(masterEnv);
		for (int i = 0; i < instance.n; ++i) {
			cut2 += instance.p[i]*y[i];
		}
		for (unsigned int i = 0; i < path.nodes.size(); ++i) {
			if (path.nodes[i].dev > 0) cut2 += path.nodes[i].devWeight*y[path.nodes[i].node]*path.nodes[i].dev;
			else break;
		}
		add(cut2 <= instance.S);
		
	}
}

int main(){

	IloEnv env;
	
	IloModel model(env);

	Instance instance("instances/1000_USA-road-d.BAY.gr",0);

	try {
		IloArray<IloBoolVarArray> x(env, instance.n);
		for (int i = 0; i < instance.n; ++i) {
			x[i] = IloBoolVarArray(env, instance.n);
			for (int j = 0; j < instance.n; j++) {
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
		//cplex.setOut(env.getNullStream());
		cplex.use(lazyCallbackCplexOpt(env,cplex,instance,x,y,z));
		cplex.setParam(IloCplex::CutsFactor, 1.0);
		cplex.setParam(IloCplex::EachCutLim, 0);
		cplex.setParam(IloCplex::PreInd, 0);
		cplex.setParam(IloCplex::MIPDisplay, 2);
		cplex.solve();

		if (cplex.getStatus() == IloAlgorithm::Infeasible)
		  cout << "No Solution" << endl;

		else {
		  cout << "objective: " << cplex.getObjValue() << endl;

		  // Afficher la valeur des variables
		  /*IloNumArray xSolution(env);
		  cplex.getValues(x, xSolution);
		  cout << "x: " <<  xSolution[0] << ", " << xSolution[1] << endl;
		  cout << "y: " << cplex.getValue(y) << endl;
		  
		  // Liberer la memoire
		  x.end();
		  y.end();*/
		}

	// ********************
	// Si une erreur survient  
	// ********************
	} catch (const IloException& e){

	// Afficher l'erreur
	cerr << e;
	throw;
	}

	model.end();
	env.end();

}
