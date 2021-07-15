# Using graphillion
from graphillion import GraphSet
import graphillion.tutorial as tl

def A271592(n, k):
    if k == 1: return 1
    universe = tl.grid(k - 1, n - 1)
    GraphSet.set_universe(universe)
    start, goal = 1, n
    paths = GraphSet.paths(start, goal, is_hamilton=True)
    return paths.len()

print([A271592(j + 1, i - j + 1) for i in range(12) for j in range(i + 1)])  