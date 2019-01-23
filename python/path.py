# -*- coding: utf-8 -*-
"""
Created on Tue Jan 22 10:55:08 2019

@author: Victor
"""

class Path:
    def __init__(self,instance,path):
        self.instance = instance
        self.path = path
        
        self.nodes = []
        self.edges = []
        
        self.isValid = True
        self.dist = 0
        self.weight = 0        
        self.length = len(path)        
        
        for i in range(len(path)-1):
            
            if not instance.hasEdge(path[i],path[i+1]):
                self.isValid = False
                break
            
            dist = instance.edgeDist(path[i],path[i+1])
            self.edges += [[dist,(path[i],path[i+1]),0]]
            self.dist += dist
            
            weight = instance.nodeWeight(path[i])
            self.nodes += [[instance.ph[path[i]],weight,path[i],0]]
            self.weight += weight
            
        weight = instance.nodeWeight(path[-1])
        self.nodes += [[instance.ph[path[-1]],weight,path[i],0]]
        self.weight += weight
        
        # worst case scenario
        self.edges.sort(reverse=True)
        self.nodes.sort(reverse=True)
    
        d1 = instance.d1
        for i in range(len(self.edges)):
            edge = self.edges[i][1]
            if d1 > instance.D[edge[0]][edge[1]]:
                dev = instance.D[edge[0]][edge[1]]
                self.edges[i][2] = dev
                d1 -= dev
            else:
                dev = d1
                self.edges[i][2] = dev
                d1 -= dev
                break
    
        d2 = instance.d2
        for i in range(len(self.nodes)):
            if d2 > 2:
                dev = 2
                self.nodes[i][3] = dev
                d2 -= dev
            else:
                dev = d2
                self.nodes[i][3] = dev
                d2 -= dev
                break    
    
        self.worstDist = self.dist
        for dist, edge, dev in self.edges:
            self.worstDist += dist*dev
            
        self.worstWeight = self.weight
        for devWeight, weight, node, dev in self.nodes:
            self.worstWeight += devWeight*dev
        
    def __str__(self):
        s = "Path : " + str(self.path[0])
        for node in self.path[1:]:
            s += " -> "
            s += str(node)
        
        s += "\n"
        s += "Distance : " + str(self.dist) + " (" + str(self.worstDist) + ")\n"
        s += "Weight : " + str(self.weight) + " (" + str(self.worstWeight) + ")"
        
        return s