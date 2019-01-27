from instance import *
from path import *     
from shortestCapacitedPath import *

instance = Instance("../instances/2500_USA-road-d.BAY.gr")

def staticNodeMetric(instance,u):
    return instance.nodeWeight(u)
    
def staticEdgeMetric(instance,u,v):
    return instance.edgeDist(u,v)

def semiWorstCaseNodeMetric(instance,u):
    return instance.nodeWeight(u) + instance.ph[u]

def worstCaseEdgeMetric(instance,u,v):
    return instance.edgeDist(u,v) * (1. + instance.D[u][v])
     

def nodes_to_remove(instance, up_bound, staticSCP_s, static_SCP_t, s, t):
    l = []
    S = instance.S
    notReacheable = 0
    notGoodEnough = 0
    for i in range(instance.n):
        if i != s and i != t:
            paretos_s = staticSCP_s.table[i].getList()
            paretos_t = staticSCP_t.table[i].getList()

            if len(paretos_s) == 0 or len(paretos_t) == 0:
                l += [i]
                #print "not reachable"
                notReacheable += 1
            else:
                for (p_s,d_s,idxs) in paretos_s: 
                    for (p_t,d_t,idxt) in paretos_t:
                        if p_s + p_t - instance.p[i] + min(2,instance.d2)*instance.ph[i] < S and d_s + d_t < up_bound:
                            break
                    else:
                        continue
                    break
                else:
                    #print "not good enough"
                    notGoodEnough += 1
                    l += [i]
    print(notReacheable, notGoodEnough)
    return l

def edges_to_remove(instance, up_bound, staticSCP_s, static_SCP_t, s, t):
    l = []
    S = instance.S
    for i in range(instance.n):
        for j in range(instance.n):
            if instance.adj[i][j] and i != s and j != t:
                paretos_s = staticSCP_s.table[i].getList()
                paretos_t = staticSCP_t.table[j].getList()
                if len(paretos_s) == 0 or len(paretos_t) == 0:
                    #print "not reachable"
                    l += [(i,j)]
                else:
                    for (p_s,d_s,idxs) in paretos_s: 
                        for (p_t,d_t,idxt) in paretos_t:
                            mini_ph = min(instance.ph[i], instance.ph[j])
                            maxi_ph = max(instance.ph[i], instance.ph[j])
                            c_max = min(2, instance.d2)
                            c_min = min(2, instance.d2-c_max)
                            if p_s + p_t + c_max*maxi_ph + c_min*mini_ph < S and d_s + d_t + instance.d[i][j]*(1+instance.D[i][j])< up_bound:
                                break
                        else:
                            continue
                        break
                    else:
                        #print "not good enough"
                        l += [(i,j)]
        
    return l

modified = True

semiWorstCaseSCP = ShortestCapacitedPath(instance,instance.s,instance.t,semiWorstCaseNodeMetric,worstCaseEdgeMetric,False,True)

supBound = semiWorstCaseSCP.shortestPath.worstDist

while modified:

    staticSCP_s = ShortestCapacitedPath(instance,instance.s,instance.t,staticNodeMetric,staticEdgeMetric,False,True)
    staticSCP_t = ShortestCapacitedPath(instance,instance.t,instance.s,staticNodeMetric,staticEdgeMetric,True,True)
    l = nodes_to_remove(instance, supBound, staticSCP_s, staticSCP_t, instance.s, instance.t)
    
    edgesR = edges_to_remove(instance, supBound, staticSCP_s, staticSCP_t, instance.s, instance.t)
    
    edgesR = set(edgesR)
    nodesR = set(l)
    for i in range(instance.n):
        if i in nodesR:
            for j in range(instance.n):
                if instance.adj[i][j]:
                    edgesR.add((i,j))
                if instance.adj[j][i]:
                    edgesR.add((j,i))
    d = len(list(edgesR))*1.
    
    c = 0.
    for i in range(instance.n):
        c += len(instance.neighbors[i])
    
    modified = len(nodesR) + len(edgesR) > 0
    instance = instance.restrict(nodesR,edgesR)
    
    print("Remaining nodes : " + str(instance.n))
    print("Remaining edges : " + str(c-d) + " (" + str(int(100*d/c)) + "% removed edges)")
    print("")