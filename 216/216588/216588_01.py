# Using graphillion

from graphillion import GraphSet

def make_CnXCk(n, k):
    grids = []
    for i in range(1, k + 1):
        for j in range(1, n):
            grids.append((i + (j - 1) * k, i + j * k))
        grids.append((i + (n - 1) * k, i))
    for i in range(1, k * n, k):
        for j in range(1, k):
            grids.append((i + j - 1, i + j))
        grids.append((i + k - 1, i))
    return grids

def A216588(n):
    universe = make_CnXCk(n, 4)
    GraphSet.set_universe(universe)
    cycles = GraphSet.cycles(is_hamilton=True)
    return cycles.len()

for n in range(3, 101):
    print(str(n) + " " + str(A216588(n)))
