# -*- coding: utf-8 -*-
"""
Created on Sun Jan 20 22:42:05 2019

@author: Victor
"""

import graph
import io

class Instance:
    def nodeWeight(self,i):
        return self.p[i]
    
    def edgeDist(self,i,j):
        return self.d[i][j]    
    
    def hasEdge(self,i,j):
        return self.adj[i][j]
    
    def __init__(self, filename):
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
                
            self.adj = [[0 for i in range(self.n)] for j in range(self.n)]
            self.d = [[0 for i in range(self.n)] for j in range(self.n)]
            self.D = [[0. for i in range(self.n)] for j in range(self.n)]
            self.neighbors = [[] for i in range(self.n)]
            #self.graph = graph.Graph(True)
            
            #for i in range(self.n):
             #   self.graph.createNode(p = self.p[i], ph = self.ph[i])
                
            for line in lines[9:]:
                L = line[:-2].split(" ")
                u = int(L[0])-1
                v = int(L[1])-1
                self.adj[u][v] = True
                self.d[u][v] = int(L[2])
                self.D[u][v] = float(L[3])
                self.neighbors[u] += [v]
                #self.graph.addEdge(u, v, d = self.d[u][v], D = self.D[u][v])