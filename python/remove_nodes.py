from instance import *
from path import *     
from shortestCapacitedPath import *

def staticNodeMetric(instance,u):
	return instance.nodeWeight(u)
    
def staticEdgeMetric(instance,u,v):
	return instance.edgeDist(u,v)

def semiWorstCaseNodeMetric(instance,u):
	return instance.nodeWeight(u) + instance.ph[u]

def worstCaseEdgeMetric(instance,u,v):
	return instance.edgeDist(u,v) * (1. + instance.D[u][v])
    

def preprocessNodes(instance, up_bound, staticSCP_s, static_SCP_t, s, t):
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
						if p_s + p_t - instance.p[i] + min(2,instance.d2)*instance.ph[i] < S and d_s + d_t < up_bound:
							break
					else:
						continue
					break
				else:
					l += [i]
	return l

def preprocessEdges(instance, up_bound, staticSCP_s, static_SCP_t, s, t):
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
						l += [(i,j)]
		
	return l

	
def preprocessInstance(instance, supBound):
	modified = True

	if supBound == None:
		semiWorstCaseSCP = ShortestCapacitedPath(instance,instance.s,instance.t,semiWorstCaseNodeMetric,worstCaseEdgeMetric,False,False)
		supBound = semiWorstCaseSCP.shortestPath.worstDist

	while modified:

		staticSCP_s = ShortestCapacitedPath(instance,instance.s,instance.t,staticNodeMetric,staticEdgeMetric,False,False)
		staticSCP_t = ShortestCapacitedPath(instance,instance.t,instance.s,staticNodeMetric,staticEdgeMetric,True,False)
		
		removedNodes = set(preprocessNodes(instance, supBound, staticSCP_s, staticSCP_t, instance.s, instance.t))
		removedEdges = set(preprocessEdges(instance, supBound, staticSCP_s, staticSCP_t, instance.s, instance.t))
		
		for i in range(instance.n):
			if i in removedNodes:
				for j in range(instance.n):
					if instance.adj[i][j]:
						removedEdges.add((i,j))
					if instance.adj[j][i]:
						removedEdges.add((j,i))
		d = len(list(removedEdges))*1.
		
		c = 0.
		for i in range(instance.n):
			c += len(instance.neighbors[i])
		
		modified = len(removedNodes) + len(removedEdges) > 0
		instance = instance.restrict(removedEdges,removedEdges)
		
	return instance
		# print("Remaining nodes : " + str(instance.n))
		# print("Remaining edges : " + str(c-d) + " (" + str(int(100*d/c)) + "% removed edges)")
		# print("")