#include "shortest_capacited_path.h"



/*class ShortestCapacitedPath:
    def __init__(self,instance,initNode,endNode,nodeMetric,edgeMetric,reverse=False,scc=False):
        self.instance = instance
        self.nodeMetric = nodeMetric
        self.edgeMetric = edgeMetric

        self.table = []
        for i in range(self.instance.n):
            self.table += [SortedList()]

        self.table[initNode].addValue(self.nodeMetric(self.instance,initNode),0,None)
        if not reverse:
            if scc:
                order = []
                done = [False for i in range(instance.n)]
                inOrder = [False for i in range(instance.n)]
                sccs = sconnect(instance.neighbors,instance.s)
                for i in range(len(sccs)-1,-1,-1):
                    for j in range(len(sccs[i])-1,-1,-1):
                        order += [sccs[i][j]]

                for u in order:
                    inOrder[u] = True

                for u in range(instance.n):
                    if not inOrder[u]:
                        order += [u]

                iter = 0
                while (instance.n > sum(done)):
                    for u in order:
                        if not done[u]:
                            done[u] = True
                            for x in self.table[u].getList():
                                for v in instance.neighbors[u]:
                                    #if not done[v]:
                                    weight = x[0] + self.nodeMetric(self.instance,v)
                                    if weight <= instance.S:
                                        if self.table[v].addValue(weight,x[1] + self.edgeMetric(self.instance,u,v),u):
                                            done[v] = False
                                        iter += 1
                                        if (iter % 200000) == 0:
                                            print("Nodes to process : " + str(instance.n-sum(done)))

        else:
            if scc:
                order = []
                done = [False for i in range(instance.n)]
                inOrder = [False for i in range(instance.n)]
                sccs = sconnect(instance.predecessors,instance.s)
                for i in range(len(sccs)-1,-1,-1):
                    for j in range(len(sccs[i])-1,-1,-1):
                        order += [sccs[i][j]]

                for u in order:
                    inOrder[u] = True

                for u in range(instance.n):
                    if not inOrder[u]:
                        order += [u]

                iter = 0
                while (instance.n > sum(done)):
                    for u in order:
                        if not done[u]:
                            done[u] = True
                            for x in self.table[u].getList():
                                for v in instance.predecessors[u]:
                                    #if not done[v]:
                                    weight = x[0] + self.nodeMetric(self.instance,v)
                                    if weight <= instance.S:
                                        if self.table[v].addValue(weight,x[1] + self.edgeMetric(self.instance,v,u),u):
                                            done[v] = False
                                        iter += 1
                                        if (iter % 200000) == 0:
                                            print("Nodes to process : " + str(instance.n-sum(done)))

        self.shortestPath = self.extractPathNodes(initNode,endNode,-1)
        self.lightestPath = self.extractPathNodes(initNode,endNode,0)

    def extractPathNodes(self,s,t,pathIdx):
        nodes = [t]
        if self.table[t].getList() == []:
            return None
        weight = self.table[t].getList()[pathIdx][0] - self.nodeMetric(self.instance,t)
        u = self.table[t].getList()[pathIdx][2]

        while u != s:
            nodes = [u] + nodes
            list = self.table[u].getList()
            for x in list:
                if x[0] == weight:
                    weight -= self.nodeMetric(self.instance,u)
                    u = x[2]

        nodes = [s] + nodes
        return Path(self.instance, nodes)
*/
