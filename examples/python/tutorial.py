# MonoSAT Python Tutorial

#This is a brief introduction to MonoSAT's Z3-inspired Python 3 library, which you can use to
#conveniently construct and solve formulas with MonoSAT.

#Before going any further, see the installation instructions for the Python library in [README].
#Also, be warned that this library has only been tested with Python 3.3+, and may not work on earlier
#versions (in particular, Python 2 may not work at all).

#Using MonoSAT in Python is as simple as:

#Import the MonoSAT library
from monosat import *
#Create two Boolean variables:
a = Var() 
b = Var() 
c = Or(a, Not(b)) #An equivalent way to write this is

#Add a unit clause to the solver, asserting that variable c must be true
Assert(c)

#Solve the instance in MonoSAT, returning either True if the instance is SAT, and False if it is UNSAT
result = Solve()
if result:
	print("SAT")
	#After a satisfiable call to Solve(), you can query the asignments given by the solver to 
	#individual variables using v.value()
	print("a: " + str(a.value())) 
	print("b: " + str(b.value()))
	print("c: " + str(c.value()))
else:
	print("UNSAT")
	
#You can continue making further assertions, creating new variables, and making incremental calls to the solver
#Note that currently, as soon as you add a new clause or assertion, the solver interally resets and discards its
#any assignments to variables (so calls to value() will stop working until the next call to Solve())

d= Var()
Assert(Implies(d, Or(a,b)))
Assert(d)

result = Solve()
if result:
	print("SAT")
	print("a: " + str(a.value())) 
	print("b: " + str(b.value()))
	print("c: " + str(c.value()))
	print("d: " + str(d.value()))
else:
	print("UNSAT")

#You can use the '~' operator to apply negation, the same way as Not()
Assert(~(And(a, b)))
result = Solve()
if result:
	print("SAT")
	print("a: " + str(a.value())) 
	print("b: " + str(b.value()))
	print("c: " + str(c.value()))
	print("d: " + str(d.value()))
else:
	print("UNSAT")

#There is no way to remove assertions from MonoSAT yet, however, you can use the assumption mechanism to
#temporarily assert that a variable must be true (or false):

result = Solve([b])
if result:
	print("SAT")
	print("a: " + str(a.value())) 
	print("b: " + str(b.value()))
	print("c: " + str(c.value()))
	print("d: " + str(d.value()))
else:
	print("UNSAT (under the assumption that 'b' is True)")

#If in the previous call, MonoSAT was only UNSAT under an assumption, the solver can still be used in subsequent calls:
result = Solve([~b])
if result:
	print("SAT")
	print("a: " + str(a.value())) 
	print("b: " + str(b.value()))
	print("c: " + str(c.value()))
	print("d: " + str(d.value()))
else:
	print("UNSAT (under the assumption that 'b' is False)")

### Theory Support

#Now, onto the interesting stuff. 
#MonoSAT also has support for several useful theories, including both common ones (Bitvectors, Cardinality constraints), 
#And uncommon ones - especially, graph predicates such as reachability, shortest paths, maximum flows, and minimum spanning tree length.
#In fact, MonoSAT has support for many more theories from other domains, including geometry, finite state machines, and more,
#but the graph theory is the most well supported, currently.

#Constructing a graph in MonoSAT is as easy as:
g = Graph()

#Create three nodes
n1 = g.addNode()
n2 = g.addNode()
n3 = g.addNode()

#Add three directed edges to the graph
e1 = g.addEdge(n1,n2) 
e2 = g.addEdge(n2,n3) 
e3 = g.addEdge(n1,n3)

#e1, e2, and e3 are *symbolic edges*, meaning that the edge (n1,n2) is included in G if and only if the
#theory atom e1 is assigned to True by MonoSAT.
#You can use e1,e2, and e3 just like variables in MonoSAT, and in that way control which edges are in the graph
#using arbitrary Boolean logic:

Assert(Not(And(e1,e2,e3)))
Assert(Or(e1,e3))

#You can even mix these symbolic edge variables with other logic from MonoSAT
Assert(Implies(c, e1)) 
 
#Once you have created a graph and some edges, you can assert graph properties about that graph:
#For example, you can assert that node n3 must be reachable from node n1, in g
Assert(g.reaches(n1,n3))

result = Solve()
if result:
	print("SAT")
	print("e1: " + str(e1.value())) 
	print("e2: " + str(e2.value()))
	print("e3: " + str(e3.value()))

else:
	print("UNSAT")
	
#Graph predicates are 'double sided', so you can also assert that they are false, in order to 
#prevent one node from reaching another:
Assert(Not(g.reaches(n2,n1)))

#You can also mix graph predicates in with arbitrary logic, just like variables and edges
Assert(Or(~b, ~g.reaches(n1,n2)))

result = Solve()
if result:
	print("SAT")
	print("e1: " + str(e1.value())) 
	print("e2: " + str(e2.value()))
	print("e3: " + str(e3.value()))

else:
	print("UNSAT")

#Edges can also have weights, represented as fixed-width, bounded bitvectors.
#(By bounded bitvectors, we mean that every bitvector in MonoSAT is asserted to 
#be in the range [0, Max], and can never overflow/underflow)

#create a bitvector of width 4
bv1 = BitVector(4)
bv2 = BitVector(4)
bv3 = BitVector(4)

#BitVectors support addition and comparisons to constants, but do not yet directly support negatives 
#or subtraction (the bitvectors are unsigned)
Assert(bv1+bv2 <= 7)

Assert(bv1 + bv3 >= 3)
Assert(bv1 >= 2)

result = Solve()
if result:
	print("SAT")
	print("bv1: " + str(bv1.value())) 
	print("bv2: " + str(bv2.value()))
	print("bv3: " + str(bv3.value()))

else:
	print("UNSAT")

#When creating an edge, you can use bitvectors (or Python ints) as edge weights (otherwise, by default, every edge has weight '1'):
#Create a new graph
g2 = Graph()
#Create three nodes
n4 = g2.addNode()
n5 = g2.addNode()
n6 = g2.addNode()

#Add three weighted, undirected edges to the graph
e4 = g2.addEdge(n4,n5, bv1) 
e5 = g2.addEdge(n5,n6, bv2) 
e6 = g2.addEdge(n4,n6, bv3)

#MonoSAT supports several useful graph predicates in addition to reachability, including:
#Shortest path constraints:
#Assert that the distance from n1 to n3 is less or equal to 1 (edges have default weights of 1)
Assert(g2.distance_leq(n4,n6,3)) 

#You can also use BitVectors in the arguments of graph predicates:
bv4 = BitVector(4)
Assert(Not(g2.distance_lt(n4,n6,bv4)))
Assert(bv4 == (bv1 + bv2))

result = Solve()
if result:
	print("SAT")
	print("e4: " + str(e4.value())) 
	print("e5: " + str(e5.value()))
	print("e6: " + str(e6.value()))
	print("bv1: " + str(bv1.value())) 
	print("bv2: " + str(bv2.value()))
	print("bv3: " + str(bv3.value()))
	print("bv4: " + str(bv4.value()))
else:
	print("UNSAT")
	
	
#MonoSAT also features highly optimized support for maximum flow constraints, allowing for comparisons against either a python integer, or a bitvector:
Assert(g2.maxFlowGreaterOrEqualTo(n4,n6,3))

bv5 = BitVector(4)
Assert(g2.maxFlowGreaterOrEqualTo(n4,n6,bv5))

#Just like with reachability and shortest path constraints, these maximum flow predicates are two sided
#so you can assert that the maximum flow must be less than a given bitvector, or you can include the
#maximum flow predicate as part of arbitrary Boolean logic 
Assert(Or(~c,~g2.maxFlowGreaterOrEqualTo(n4,n6,bv5+1)))
result = Solve()
if result:
	print("SAT")
	print("e4: " + str(e4.value())) 
	print("e5: " + str(e5.value()))
	print("e6: " + str(e6.value()))
	print("bv1: " + str(bv1.value())) 
	print("bv2: " + str(bv2.value()))
	print("bv3: " + str(bv3.value()))
	print("bv5: " + str(bv5.value()))
else:
	print("UNSAT")
	
result = Solve([bv5==4])
if result:
	print("SAT")
	print("e4: " + str(e4.value())) 
	print("e5: " + str(e5.value()))
	print("e6: " + str(e6.value()))
	print("bv1: " + str(bv1.value())) 
	print("bv2: " + str(bv2.value()))
	print("bv3: " + str(bv3.value()))
	print("bv5: " + str(bv5.value()))
else:
	print("UNSAT")
	
	
result = Solve([bv5>4, bv5<7])
if result:
	print("SAT")
	print("e4: " + str(e4.value())) 
	print("e5: " + str(e5.value()))
	print("e6: " + str(e6.value()))
	print("bv1: " + str(bv1.value())) 
	print("bv2: " + str(bv2.value()))
	print("bv3: " + str(bv3.value()))
	print("bv5: " + str(bv5.value()))
else:
	print("UNSAT")
	
#MonoSAT also features good support for minimum spanning tree constraints (in undirected graphs):
g3 = Graph()
n7 = g3.addNode()
n8 = g3.addNode()
n9 = g3.addNode()

#Add three weighted, undirected edges to the graph
e7 = g3.addUndirectedEdge(n7,n8, 1) 
e8 = g3.addUndirectedEdge(n8,n9, 2) 
e9 = g3.addUndirectedEdge(n7,n9, 4)

Assert(g3.minimumSpanningTreeLessEq(3))
Assert(~g3.minimumSpanningTreeLessEq(1))

result = Solve()
if result:
	print("SAT")
	print("e7: " + str(e7.value())) 
	print("e8: " + str(e8.value()))
	print("e9: " + str(e9.value()))
else:
	print("UNSAT")

#(Minimum spanning tree constraints don't support bitvectors yet, but they could in the future)

# CSG Tutorial!

# To use the CSG theory, you must first initialize it!
csg = CSG()

p1 = csg.addPoint(0,0)
p2 = csg.addPoint(1,1)
p3 = csg.addPoint(-1,1)
p4 = csg.addPoint(-1,-1)
p5 = csg.addPoint(1,-1)
p6 = csg.addPoint(0,1)
p7 = csg.addPoint(1,0)
p8 = csg.addPoint(0,-1)
p9 = csg.addPoint(-1,0)

print("p1: " + str(p1))
print("p2: " + str(p2))
print("p3: " + str(p3))
print("p4: " + str(p4))
print("p5: " + str(p5))
print("p6: " + str(p6))
print("p7: " + str(p7))
print("p8: " + str(p8))
print("p9: " + str(p9))

plane1 = csg.addPlane(p7,p9)
plane2 = csg.addPlane(p6,p8)
plane3 = csg.addPlane(p9,p7)
plane4 = csg.addPlane(p8,p6)
plane5 = csg.addPlane(p1,p2)
plane6 = csg.addPlane(p1,p4)
plane7 = csg.addPlane(p1,p3)

print("plane1: " + str(plane1))
print("plane2: " + str(plane2))
print("plane3: " + str(plane3))
print("plane4: " + str(plane4))
print("plane5: " + str(plane5))
print("plane6: " + str(plane6))
print("plane7: " + str(plane7))

shape1 = csg.addPrimative([plane1, plane2, plane5])
shape2 = csg.addPrimative([plane3, plane4, plane6])
shape3 = csg.addPrimative([plane1, plane2, plane3, plane4])
shape4 = csg.addShape(shape1,shape2,0)
shape5 = csg.addConditionalPrimative([plane3, plane2, plane7])
shape6 = csg.addConditionalShape(shape1,shape2,0)
shape7 = csg.addShape(shape4,shape6,2)

print("shape1: " + str(shape1))
print("shape2: " + str(shape2))
print("shape3: " + str(shape3))
print("shape4: " + str(shape4))
print("shape5: " + str(shape5))
print("shape6: " + str(shape6))