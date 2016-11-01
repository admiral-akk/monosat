#The MIT License (MIT)
#
#Copyright (c) 2016, Kuba Karpierz
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
#associated documentation files (the "Software"), to deal in the Software without restriction,
#including without limitation the rights to use, copy, modify, merge, publish, distribute,
#sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all copies or
#substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
#NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
#DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
#OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import monosat.monosat_c
import sys
from monosat.bvtheory import BitVector
from monosat.logic import *
from monosat.manager import Manager
class GeometryManager(metaclass=Manager):

    def  __init__(self):
        self.points = []
        self.shapes = []
        self.planes = []   
        self.csg = 0
    
    def clear(self):
        self.points = []
        self.planes = []
        self.shapes = []   
    
    def addPoint(self, p):
        self.points.append(p)
    
    def newPolygon(self,polygon):
        self.polygons.append(polygon)
    
    def addCSG(self, csg):
        self.csg = csg
    
class CSG():
    class CSGType():
        int=1
        float=2
        rational=3

    def __init__(self,CSG_Type=1):
        self._monosat = monosat.monosat_c.Monosat()
        manager = GeometryManager()
        manager.addCSG(self)
        self.csg = self._monosat.initCSGTheory()

        self.points = []
        self.planes = []
        self.shapes = []   


    def addPoint(self, x, y):  
        n = self._monosat.newPoint(self.csg, x, y)        
        self.points.append([x,y])        
        return n

    def addPlane(self, point, vector):  
        n = self._monosat.newPlane(self.csg, point, vector)    
        self.planes.append([point, vector])        
        return n

    def addPrimative(self, planeArr):  
        n = self._monosat.newPrimative(self.csg, planeArr)    
        self.shapes.append(n)        
        return n

    def addConditionalPrimative(self, planeArr):  
        n = self._monosat.newConditionalPrimative(self.csg, planeArr)    
        self.shapes.append(n)        
        return n

    def addShape(self, A, B, typeIndex):  
        n = self._monosat.newShape(self.csg, A, B, typeIndex)    
        self.shapes.append(n)        
        return n

    def addConditionalShape(self, A, B, typeIndex):  
        n = self._monosat.newConditionalShape(self.csg, A, B, typeIndex)    
        self.shapes.append(n)        
        return n

