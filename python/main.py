# -*- coding: utf-8 -*-
"""
Created on Sun Jan 20 23:02:02 2019

@author: Victor
"""
from os import listdir
from os.path import isfile, join 

from instance import *
from path import *     
from shortestCapacitedPath import *
from removed_nodes import *

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
    return instance.nodeWeight(u) + instance.ph[u]
    
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
for file in files:
	print("Processing " + file + " ...")
	instance = Instance("../instances/" + file)
	penalty = 2.
	bestBound = 10000000000000
	feasibleFound = False
	
	for i in range(3):
		nodeMetric = lambda instance,u : parameterizedWorstCaseNodeMetric(instance,u,penalty)
		edgeMetric = lambda instance,u,v : parameterizedWorstCaseEdgeMetric(instance,u,v,penalty)
		scp = ShortestCapacitedPath(instance,instance.s,instance.t,nodeMetric,edgeMetric,False,False)
		
		for i in range(scp.table[instance.t].size()):
			path = scp.extractPathNodes(instance.s,instance.t,i)
			if path.worstWeight <= instance.S:
				if bestBound > path.worstDist:
					bestBound = path.worstDist
					feasibleFound = True
			
		penalty = penalty/2.
	
	while not feasibleFound:
		nodeMetric = lambda instance,u : parameterizedWorstCaseNodeMetric(instance,u,penalty)
		edgeMetric = lambda instance,u,v : parameterizedWorstCaseEdgeMetric(instance,u,v,penalty)
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
	
	removedNodes = instance.n - initialNodes
	pRemovedNodes = int(1000*removedNodes/instance.n)/1000
	removedEdges = instance.m - initialEdges
	pRemovedEdges = int(1000*removeEdges/instance.m)/1000
	
	print("Done. [Bound = " + str(bestBound) + ", " + str() + " nodes removed " + str(removedNodes) + " (" + str(pRemovedNodes) + "), edges removed " + str(removedEdges) + " (" + str(pRemovedEdges) + ")].")
	

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

