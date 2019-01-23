# -*- coding: utf-8 -*-
"""
Created on Tue Jan 22 10:56:12 2019

@author: Victor
"""

class SortedList:
    def __init__(self):
        self.values = []
        self.preds = []
        self.weights = []
        
        
    def addValue(self,weight,val,pred):
        i = len(self.weights)-1
        i_ = None
        while i >= 0 and weight < self.weights[i]:
            if self.values[i] >= val:
                i_ = i
                break
            i -= 1
        
        while i >= 0 and weight < self.weights[i]:
            i -= 1
            
        if i_ == None:
            i_ = i
            
        if i < 0:
            self.weights = [weight] + self.weights[i_+1:]
            self.values = [val] + self.values[i_+1:]
            self.preds = [pred] + self.preds[i_+1:]
            
        else:
            if self.values[i] > val:
                if self.weights[i] == weight:
                    j = i
                else:
                    j = i + 1
                
                self.weights = self.weights[:j] + [weight] + self.weights[i_+1:]
                self.values = self.values[:j] + [val] + self.values[i_+1:]
                self.preds = self.preds[:j] + [pred] + self.preds[i_+1:]
            else:
                return False
                
        return True
        
    def getList(self):
        return [(self.weights[i],self.values[i],self.preds[i]) for i in range(len(self.weights))]

    def empty(self):
        return len(self.weights) == 0

    def size(self):
        return len(self.weights)
        
    def __getitem__(self,i):
        return (self.weights[i],self.values[i],self.preds[i])