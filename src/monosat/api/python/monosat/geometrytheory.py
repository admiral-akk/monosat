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
    
class CSG():
    class CSGType():
        int=1
        float=2
        rational=3

    def __init__(self,CSG_Type=1):
        self._monosat = monosat.monosat_c.Monosat()
        self.csg = self._monosat.initCSGTheory()

        self.points = []
        self.planes = []
        self.shapes = []
        self.pointContains = []   


    def addPoint(self, x, y):  
        self._monosat.newPoint(self.csg, x, y)        
        self.points.append([x,y])        
        return len(self.points) - 1

    def addPlane(self, point, vector):  
        self._monosat.newPlane(self.csg, point, vector)    
        self.planes.append([point, vector])        
        return len(self.planes) - 1

    def addPrimative(self, planeArr):  
        self._monosat.newPrimative(self.csg, planeArr)    
        self.shapes.append(planeArr)          
        return len(self.shapes) - 1

    def addConditionalPrimative(self, planeArr):  
        var = Var(self._monosat.newConditionalPrimative(self.csg, planeArr)) 
        self.shapes.append(planeArr)          
        return var

    def addShape(self, A, B, typeIndex):  
        self._monosat.newShape(self.csg, A, B, typeIndex)    
        self.shapes.append([-1, A, B, typeIndex])        
        return len(self.shapes) - 1

    def addConditionalShape(self, A, B, typeIndex):  
        var = Var(self._monosat.newConditionalShape(self.csg, A, B, typeIndex))
        self.shapes.append([-1, A, B, typeIndex])        
        return var

    def addShapeContainsPoint(self, shape, point):  
        var = Var(self._monosat.newShapeContainsPoint(self.csg, shape, point))   
        self.pointContains.append([shape, point])      
        return var
