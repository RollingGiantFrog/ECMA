# -*- coding: utf-8 -*-
"""
Created on Sun Jan 20 23:02:02 2019

@author: Victor
"""

from instance import *
from path import *     
from shortestCapacitedPath import *

print("Loading instance...")

#instance = Instance("instances/20_USA-road-d.COL.gr")
#instance = Instance("instances/180_USA-road-d.BAY.gr")
#instance = Instance("instances/400_USA-road-d.BAY.gr")
instance = Instance("instances/800_USA-road-d.NY.gr")
#instance = Instance("instances/2400_USA-road-d.COL.gr")
#instance = Instance("instances/2500_USA-road-d.NY.gr")

#instance = Instance("instances/1000_USA-road-d.BAY.gr")

print("Done.")

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
    
print("Static SCP resolution")
staticSCP = ShortestCapacitedPath(instance,instance.s,instance.t,staticNodeMetric,staticEdgeMetric,True)
print("Static SCP done.")
#print("\nWorst case SCP resolution")
#worstCaseSCP = ShortestCapacitedPath(instance,worstCaseNodeMetric,worstCaseEdgeMetric)
#print("Worst case SCP done.")
print("\nSemi worst case SCP resolution")
semiWorstCaseSCP = ShortestCapacitedPath(instance,instance.s,instance.t,semiWorstCaseNodeMetric,worstCaseEdgeMetric,True)
print("Semi worst case SCP done.")
#print("\nQuadratic edges SCP resolution")
#quadraticEdgeSCP = ShortestCapacitedPath(instance,worstCaseNodeMetric,quadraticEdgeMetric)
#print("nQuadratic edges SCP done.")

print("")

# compte le plus court chemin en nombre d'arêtes pour aller de s à t
leastEdgeSCP = ShortestCapacitedPath(instance,instance.s,instance.t,staticNodeMetric,leastEdgeMetric,True)
print(leastEdgeSCP.table[instance.t].getList()[-1][1]-2)
leastDeviationSCP = ShortestCapacitedPath(instance,instance.s,instance.t,staticNodeMetric,deviationMetric,True)
print(leastDeviationSCP.table[instance.t].getList()[-1][1])
    
mDs = min([e for e in instance.D[instance.s] if e > 0])
mds = min([e for e in instance.d[instance.s] if e > 0])

mDt = 100000000
mdt = 100000000
for i in range(instance.n):
    if instance.adj[i][instance.t]:
        mDt = min(mDt,instance.D[i][instance.t])
        mdt = min(mdt,instance.d[i][instance.t])
    
mD = 10000000
md = 10000000
for i in range(instance.n):
    mD = min(mD,min([e for e in instance.D[i] if e > 0]))
    md = min(md,min([e for e in instance.d[i] if e > 0]))
    
if mdt < mds:
    devt = min(mDt,instance.d1)
    devs = min(mDs,instance.d1 - devt)
else:
    devs = min(mDs,instance.d1)
    devt = min(mDt,instance.d1 - devs)
dev = min(instance.d1 - devt - devs,mD*(leastEdgeSCP.table[instance.t].getList()[-1][1]-2))
dev = min(instance.d1,leastDeviationSCP.table[instance.t].getList()[-1][1]) - devs - devt

if leastDeviationSCP.table[instance.t].getList()[-1][1] > instance.d1 + devs + devt:
    infBound = staticSCP.shortestPath.dist + min(leastDeviationSCP.table[instance.t].getList()[-1][1],instance.d1)*md
else:
    infBound = staticSCP.shortestPath.dist + devt*mdt + devs*mds + dev*md


print("Shortest path = " + str(staticSCP.shortestPath.dist))

print("Inf bound = " + str(infBound))

#if (worstCaseSCP.shortestPath != None and worstCaseSCP.shortestPath.worstWeight <= instance.S):
#    print("Feasible solution (worst case shortest path) = " + str(worstCaseSCP.shortestPath.worstDist) + " (" + str(int(1000*(1-infBound/worstCaseSCP.shortestPath.worstDist))/10.) + "%)")
#if (worstCaseSCP.lightestPath != None and worstCaseSCP.lightestPath.worstWeight <= instance.S):
#    print("Feasible solution (worst case lightest path) = " + str(worstCaseSCP.lightestPath.worstDist) + " (" + str(int(1000*(1-infBound/worstCaseSCP.lightestPath.worstDist))/10.) + "%)")

if (semiWorstCaseSCP.shortestPath != None and semiWorstCaseSCP.shortestPath.worstWeight <= instance.S):
    print("Feasible solution (semi worst case shortest path) = " + str(semiWorstCaseSCP.shortestPath.worstDist) + " (" + str(int(1000*(1-infBound/semiWorstCaseSCP.shortestPath.worstDist))/10.) + "%)")
if (semiWorstCaseSCP.lightestPath != None and semiWorstCaseSCP.lightestPath.worstWeight <= instance.S):
    print("Feasible solution (semi worst case lightest path) = " + str(semiWorstCaseSCP.lightestPath.worstDist) + " (" + str(int(1000*(1-infBound/semiWorstCaseSCP.lightestPath.worstDist))/10.) + "%)")

#if (quadraticEdgeSCP.shortestPath != None and quadraticEdgeSCP.shortestPath.worstWeight <= instance.S):
#    print("Feasible solution (quadratic edges shortest path) = " + str(quadraticEdgeSCP.shortestPath.worstDist) + " (" + str(int(1000*(1-infBound/quadraticEdgeSCP.shortestPath.worstDist))/10.) + "%)")
#if (quadraticEdgeSCP.lightestPath != None and quadraticEdgeSCP.lightestPath.worstWeight <= instance.S):
#    print("Feasible solution (quadratic edges lightest path) = " + str(quadraticEdgeSCP.lightestPath.worstDist) + " (" + str(int(1000*(1-infBound/quadraticEdgeSCP.lightestPath.worstDist))/10.) + "%)")
    

#staticS = ShortestCapacitedPath(instance,instance.s,instance.t,staticNodeMetric,staticEdgeMetric,False)
#staticT = ShortestCapacitedPath(instance,instance.t,instance.s,staticNodeMetric,staticEdgeMetric,False)
#
#supBound = semiWorstCaseSCP.shortestPath.worstDist
#
#nodeCount = 0
#for u in range(instance.n):
#    if len(staticS.table[u].getList()) == 0 or  len(staticT.table[u].getList()) == 0 or staticS.table[u].getList()[-1][1] + staticT.table[u].getList()[-1][1] >= supBound:
#        print(u)
#        nodeCount += 1
#        
#edgeCount = 0
#for u in range(instance.n):
#    for v in instance.neighbors[u]:
#        if  (len(staticS.table[u].getList()) == 0 or  len(staticT.table[v].getList()) == 0 or staticS.table[u].getList()[-1][1] + staticT.table[v].getList()[-1][1] >= supBound) and ( len(staticS.table[v].getList()) == 0 or  len(staticT.table[u].getList()) == 0 or staticS.table[v].getList()[-1][1] + staticT.table[u].getList()[-1][1] >= supBound):
#            print(u,v)
#            edgeCount += 1