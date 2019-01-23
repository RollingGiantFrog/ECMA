from instance import *
from path import *     
from shortestCapacitedPath import *

instance = Instance("../instances/2500_USA-road-d.NY.gr")

def staticNodeMetric(instance,u):
    return instance.nodeWeight(u)
    
def staticEdgeMetric(instance,u,v):
    return instance.edgeDist(u,v)

def semiWorstCaseNodeMetric(instance,u):
    return instance.nodeWeight(u) + instance.ph[u]

def worstCaseEdgeMetric(instance,u,v):
    return instance.edgeDist(u,v) * (1. + instance.D[u][v])
    

semiWorstCaseSCP = ShortestCapacitedPath(instance,instance.s,instance.t,semiWorstCaseNodeMetric,worstCaseEdgeMetric,False,True)

up_bound = semiWorstCaseSCP.shortestPath.worstDist

 

def nodes_to_remove(instance, up_bound, staticSCP_s, static_SCP_t, s, t):
    l = []
    S = instance.S
    for i in range(instance.n):
        if i != s and i != t:
            paretos_s = staticSCP_s.table[i].getList()
            paretos_t = staticSCP_t.table[i].getList()

            if len(paretos_s) == 0 or len(paretos_t) == 0:
                l += [i]
            else:
                for (p_s,d_s,idxs) in paretos_s: 
                    for (p_t,d_t,idxt) in paretos_t:
                        if p_s + p_t - instance.p[i] + 2*instance.ph[i] < S and d_s + d_t < up_bound:
                            break
                    else:
                        continue
                    break
                else:
                    l += [i]
            
    return l

staticSCP_s = ShortestCapacitedPath(instance,instance.s,instance.t,staticNodeMetric,staticEdgeMetric,False,True)
staticSCP_t = ShortestCapacitedPath(instance,instance.t,instance.s,staticNodeMetric,staticEdgeMetric,True,True)
l = nodes_to_remove(instance, up_bound, staticSCP_s, staticSCP_t, instance.s, instance.t)
print(instance.s)

for i in semiWorstCaseSCP.shortestPath.path :
    assert i not in l

print(len(l))