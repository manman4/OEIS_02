# Using graphillion
from graphillion import GraphSet
import graphillion.tutorial as tl

def A231829(n, k):
    universe = tl.grid(n, k)
    GraphSet.set_universe(universe)
    cycles = GraphSet.cycles()
    return cycles.len()

print([A231829(j + 1, i - j + 1) for i in range(6) for j in range(i + 1)])  
