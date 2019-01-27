# -*- coding: utf-8 -*-
"""
Created on Tue Jan 22 10:57:09 2019

@author: Victor
"""

from path import *
from sortedList import *

def sconnect(neighbors, v):
    N = len(neighbors)
    next = 0 # Next index.
    index = [None] * N
    lowlink = [None] * N
    onstack = [False] * N
    stack = []
    nextgroup = 0 # Next SCC ID.
    groups = [] # SCCs: list of vertices.
    groupid = {} # Map from vertex to SCC ID.
    
    work = [(v, 0)] # NEW: Recursion stack.
    while work:
        v, i = work[-1] # i is next successor to process.
        del work[-1]
        if i == 0: # When first visiting a vertex:
            index[v] = next
            lowlink[v] = next
            next += 1
            stack.append(v)
            onstack[v] = True
        recurse = False
        for j in range(i, len(neighbors[v])):
            w = neighbors[v][j]
            if index[w] == None:
                # CHANGED: Add w to recursion stack.
                work.append((v, j+1))
                work.append((w, 0))
                recurse = True
                break
            elif onstack[w]:
                lowlink[v] = min(lowlink[v], index[w])
        if recurse: continue # NEW
        if index[v] == lowlink[v]:
            com = []
            while True:
                w = stack[-1]
                del stack[-1]
                onstack[w] = False
                com.append(w)
                groupid[w] = nextgroup
                if w == v: break
            groups.append(com)
            nextgroup += 1
        if work: # NEW: v was recursively visited.
            w = v
            v, _ = work[-1]
            lowlink[v] = min(lowlink[v], lowlink[w])
            
    return groups

class ShortestCapacitedPath:
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
                sccs = sconnect(instance.neighbors,initNode)
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
                queue = [initNode]
                self.iter = 0
                while len(queue) > 0:
                    u = queue.pop()
                    for v in self.instance.neighbors[u]:
                        modified = False
                        for x in self.table[u].getList():
                            weight = x[0] + self.nodeMetric(self.instance,v)
                            if weight <= self.instance.S:
                                modified = modified or self.table[v].addValue(weight,x[1] + self.edgeMetric(self.instance,u,v),u)               
                                self.iter += 1
                                if (self.iter % 1000000) == 0:
                                    print("Nodes to process : " + str(len(queue)))
                        if modified:
                            queue.append(v)
        
        else:
            if scc:
                order = []
                done = [False for i in range(instance.n)]
                inOrder = [False for i in range(instance.n)]
                sccs = sconnect(instance.predecessors,initNode)
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
    
            else:
                queue = [initNode]
                self.iter = 0
                while len(queue) > 0:
                    u = queue.pop()
                    for v in self.instance.predecessors[u]:
                        modified = False
                        for x in self.table[u].getList():
                            weight = x[0] + self.nodeMetric(self.instance,v)
                            if weight <= self.instance.S:
                                modified = modified or self.table[v].addValue(weight,x[1] + self.edgeMetric(self.instance,u,v),u)               
                                self.iter += 1
                                if (self.iter % 1000000) == 0:
                                    print("Nodes to process : " + str(len(queue)))
                        if modified:
                            queue.append(v)
                            
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
