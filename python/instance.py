# -*- coding: utf-8 -*-
"""
Created on Sun Jan 20 22:42:05 2019

@author: Victor
"""

import io

class Instance:
    def nodeWeight(self,i):
        return self.p[i]
    
    def edgeDist(self,i,j):
        return self.d[i][j]    
    
    def hasEdge(self,i,j):
        return self.adj[i][j]
    
    def __init__(self, filename=None):
        if type(filename) == str:
            with io.open(filename,'r') as f:
                lines = f.readlines()
                
                l = lines[0].replace(" ","")
                self.n = int(l.split("=")[1])
                l = lines[1].replace(" ","")
                self.s = int(l.split("=")[1]) - 1
                l = lines[2].replace(" ","")
                self.t = int(l.split("=")[1]) - 1
                l = lines[3].replace(" ","")
                self.S = int(l.split("=")[1])
                l = lines[4].replace(" ","")
                self.d1 = int(l.split("=")[1])
                l = lines[5].replace(" ","")
                self.d2 = int(l.split("=")[1])
                
                l = lines[6].replace(" ","")
                self.p = []
                for i in l[3:-2].split(","):
                    self.p += [int(i)]
                
                l = lines[7].replace(" ","")
                self.ph = []
                for i in l[4:-2].split(","):
                    self.ph += [int(i)]
                    
                self.initialNodes = range(self.n)
                    
                self.adj = [[0 for i in range(self.n)] for j in range(self.n)]
                self.d = [[0 for i in range(self.n)] for j in range(self.n)]
                self.D = [[0. for i in range(self.n)] for j in range(self.n)]
                self.neighbors = [[] for i in range(self.n)]
                self.predecessors = [[] for i in range(self.n)]
                    
                for line in lines[9:]:
                    L = line[:-2].split(" ")
                    u = int(L[0])-1
                    v = int(L[1])-1
                    self.adj[u][v] = True
                    self.d[u][v] = int(L[2])
                    self.D[u][v] = float(L[3])
                    self.neighbors[u] += [v]
                    self.predecessors[v] += [u]
                
    def restrict(self, removedNodes, removedEdges):
        instance = Instance()
        instance.n = 0
        instance.S = self.S
        instance.d1 = self.d1
        instance.d2 = self.d2
        instance.p = []
        instance.ph = []
        instance.initialNodes = []
        
        instanceToSelf = []        
        selfToInstance = [None for i in range(self.n)]        
        
        for i in range(self.n):
            if not i in removedNodes:
                selfToInstance[i] = instance.n
                instanceToSelf += [i]
                
                instance.n += 1
                instance.p += [self.p[i]]
                instance.ph += [self.ph[i]]
                instance.initialNodes += [self.initialNodes[i]]
        
        instance.s = selfToInstance[self.s]
        instance.t = selfToInstance[self.t]        
        
        
        instance.adj = [[0 for i in range(instance.n)] for j in range(instance.n)]
        instance.d = [[0 for i in range(instance.n)] for j in range(instance.n)]
        instance.D = [[0. for i in range(instance.n)] for j in range(instance.n)]
        instance.neighbors = [[] for i in range(instance.n)]
        instance.predecessors = [[] for i in range(instance.n)]
        
        for i in range(instance.n):
            for v in self.neighbors[instanceToSelf[i]]:
                if not v in removedNodes and not (instanceToSelf[i],v) in removedEdges:
                    instance.neighbors[i] += [selfToInstance[v]]
                    instance.predecessors[selfToInstance[v]] += [i]
                    instance.adj[i][selfToInstance[v]] = True
                    instance.d[i][selfToInstance[v]] = self.d[instanceToSelf[i]][v]
                    instance.D[i][selfToInstance[v]] = self.D[instanceToSelf[i]][v]
        
        return instance