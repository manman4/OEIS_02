# Using graphillion
from graphillion import GraphSet

def make_CnXCk(n, k):
    grids = []
    # 縦
    for i in range(1, k + 1):
        for j in range(1, n):
            grids.append((i + (j - 1) * k, i + j * k))
        grids.append((i + (n - 1) * k, i))
    # 横
    for i in range(1, k * n, k):
        for j in range(1, k):
            grids.append((i + j - 1, i + j))
        grids.append((i + k - 1, i))
    return grids

# print(make_CnXCk(3, 2))
# print(make_CnXCk(4, 3))


def A(n, k):
    if n == 1: return k
    if k == 1: return n
    universe = make_CnXCk(n, k)
    GraphSet.set_universe(universe)
    spanning_trees = GraphSet.trees(is_spanning=True)
    return spanning_trees.len()

print("1 4")
print("2 2304")
for n in range(3, 25):
    print(str(n) + " " + str(A(4, n)))

# def make_PnXPk(n, k):
#     grids = []
#     # 縦
#     for i in range(1, k + 1):
#         for j in range(1, n):
#             grids.append((i + (j - 1) * k, i + j * k))
#     # 横
#     for i in range(1, k * n, k):
#         for j in range(1, k):
#             grids.append((i + j - 1, i + j))
#     return grids

# cycles = GraphSet.paths(1, n * n)
# return cycles.len()