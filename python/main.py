# -*- coding: utf-8 -*-
"""
Created on Sun Jan 20 23:02:02 2019

@author: Victor
"""
from os import listdir
from os.path import isfile, join 
import io

from instance import *
from path import *     
from shortestCapacitedPath import *

print("Loading instance...")

#instance = Instance("../instances/20_USA-road-d.COL.gr")
#instance = Instance("../instances/100_USA-road-d.BAY.gr")
#instance = Instance("../instances/180_USA-road-d.BAY.gr")
#instance = Instance("../instances/400_USA-road-d.BAY.gr")
#instance = Instance("../instances/800_USA-road-d.NY.gr")
#instance = Instance("../instances/2400_USA-road-d.COL.gr")
#instance = Instance("../instances/2500_USA-road-d.BAY.gr")

instance = Instance("../instances/1100_USA-road-d.NY.gr")

print("Done.")
=======
from remove_nodes import *
>>>>>>> 49ba347ddf4ca6beb38a98db74da19e9017e58ff

def noneNodeMetric(instance,u):
    return 0

def leastEdgeMetric(instance,u,v):
    return 1

def staticNodeMetric(instance,u):
    return instance.nodeWeight(u)
    
def staticEdgeMetric(instance,u,v):
    return instance.edgeDist(u,v)

def deviationMetric(instance,u,v):
    return instance.D[u][v]

def semiWorstCaseNodeMetric(instance,u):
    return instance.nodeWeight(u) + 2*instance.ph[u]
    
def worstCaseNodeMetric(instance,u):
    return instance.nodeWeight(u) + 2*instance.ph[u]
    
def worstCaseEdgeMetric(instance,u,v):
    return instance.edgeDist(u,v) * (1. + instance.D[u][v])

def quadraticEdgeMetric(instance,u,v):
    return (instance.edgeDist(u,v))**4
    
def parameterizedWorstCaseNodeMetric(instance,u,penalty):
    return instance.nodeWeight(u) + penalty*instance.ph[u]
    
def parameterizedWorstCaseEdgeMetric(instance,u,v,penalty):
    return instance.edgeDist(u,v) * (1. + penalty*instance.D[u][v])
    
files = [f for f in listdir("../instances/") if isfile(join("../instances/", f))]
for k in range(len(files)):
	file = files[k]
	print("Processing " + file + " ... (" + str(k+1) + "/" + str(len(files)) + ")")
	instance = Instance("../instances/" + file)
	penalty = 2.
	bestBound = 10000000000000
	feasibleFound = False
	
	for i in range(3):
		print("Trying with penalty = " + str(penalty))
		nodeMetric = lambda instance,u : parameterizedWorstCaseNodeMetric(instance,u,penalty)
		edgeMetric = worstCaseEdgeMetric
		scp = ShortestCapacitedPath(instance,instance.s,instance.t,nodeMetric,edgeMetric,False,False)
		
		for i in range(scp.table[instance.t].size()):
			path = scp.extractPathNodes(instance.s,instance.t,i)
			if path.worstWeight <= instance.S:
				if bestBound > path.worstDist:
					bestBound = path.worstDist
					feasibleFound = True
			
		penalty = penalty/2.
	
	while not feasibleFound:
		print("Trying with penalty = " + str(penalty))
		nodeMetric = lambda instance,u : parameterizedWorstCaseNodeMetric(instance,u,penalty)
		edgeMetric = worstCaseEdgeMetric
		scp = ShortestCapacitedPath(instance,instance.s,instance.t,nodeMetric,edgeMetric,False,False)
		
		for i in range(scp.table[instance.t].size()):
			path = scp.extractPathNodes(instance.s,instance.t,i)
			if path.worstWeight <= instance.S:
				if bestBound > path.worstDist:
					bestBound = path.worstDist
					feasibleFound = True
			
		penalty = penalty/2.
		
	
	initialNodes = instance.n
	initialEdges = instance.m
	
	instance = preprocessInstance(instance,bestBound)
	
	instance.write("../preprocessed_instances/" + str(file))
	
	removedNodes = initialNodes - instance.n
	pRemovedNodes = int(1000*removedNodes/initialNodes)/1000.
	removedEdges = initialEdges - instance.m
	pRemovedEdges = int(1000*removedEdges/initialEdges)/1000.
	
	print("Done. [Bound = " + str(bestBound) + ", " + str() + " nodes removed " + str(removedNodes) + " (" + str(pRemovedNodes) + "), edges removed " + str(removedEdges) + " (" + str(pRemovedEdges) + ")].")
	print("")
	
	with io.open("../results_heuristic_worst_case_dist.csv",'a') as f:
		f.write(file + u";" + str(bestBound) + "\n")
	
	

# print("Loading instance...")

# instance = Instance("../instances/1000_USA-road-d.BAY.gr")

# print("Done.")

# print("Static SCP resolution")
# staticSCP = ShortestCapacitedPath(instance,instance.s,instance.t,staticNodeMetric,staticEdgeMetric,False,True)
# print("Static SCP done.")
# print("\nSemi worst case SCP resolution")
# semiWorstCaseSCP = ShortestCapacitedPath(instance,instance.s,instance.t,semiWorstCaseNodeMetric,worstCaseEdgeMetric,False,True)
# print("Semi worst case SCP done.")

# print("")

# infBound = staticSCP.shortestPath.dist
# print("Shortest path = " + str(staticSCP.shortestPath.dist))

# print("Inf bound = " + str(infBound))

# if (semiWorstCaseSCP.shortestPath != None and semiWorstCaseSCP.shortestPath.worstWeight <= instance.S):
    # print("Feasible solution (semi worst case shortest path) = " + str(semiWorstCaseSCP.shortestPath.worstDist) + " (" + str(int(1000*(1-infBound/semiWorstCaseSCP.shortestPath.worstDist))/10.) + "%)")
# if (semiWorstCaseSCP.lightestPath != None and semiWorstCaseSCP.lightestPath.worstWeight <= instance.S):
    # print("Feasible solution (semi worst case lightest path) = " + str(semiWorstCaseSCP.lightestPath.worstDist) + " (" + str(int(1000*(1-infBound/semiWorstCaseSCP.lightestPath.worstDist))/10.) + "%)")

