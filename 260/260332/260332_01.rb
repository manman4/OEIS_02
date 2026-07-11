# Exact recurrence from Theorem 4 of
# "Pattern Avoidance in Task-Precedence Posets" for the 231 case.
#
# This is a proven recurrence, not the conjectural 5-Schroeder formula.
# It computes |D_{4,n}(231)| for n >= 1, with a(0) = 1 added for the
# OEIS offset.
#
# For v = 4, write A[d][j] = alpha^d_{4,j}(1), where j = 1,2,3 is a
# final incomplete diamond with j vertices, and j = 4 is d full diamonds.
# Thus each d has four values:
#   u(d) = A[d][1]: auxiliary state with 1 vertex in the final diamond,
#   b(d) = A[d][2]: auxiliary state with 2 vertices in the final diamond,
#   c(d) = A[d][3]: auxiliary state with 3 vertices in the final diamond,
#   s(d) = A[d][4]: the target value a(d), with d complete diamonds.
#
# The recurrence splits a 231-avoiding labeling at the largest label m.  If a
# value on the left of m were larger than a value on the right of m, then
# (left, m, right) would form a 231 pattern.  Hence the two sides can be counted
# independently, producing the convolution sums below.
#
# Specializing Theorem 4 to v = 4 and x = 1 gives:
#   u(d) = s(d-1) + Sum_{i=1..d-1} c(i)*u(d-i)
#   b(d) = u(d)   + Sum_{i=1..d-1} c(i)*b(d-i)
#   c(d) = b(d) + u(d) + Sum_{i=1..d-1} c(i)*c(d-i)
#   s(d) = c(d)   + Sum_{i=1..d-1} c(i)*s(d-i)


def catalan(n)
  c = 1
  0.upto(n - 1){|i|
    c = c * 2 * (2 * i + 1) / (i + 2)
  }
  c
end

def a260332(limit)
  return [1] if limit == 0

  v = 4
  a = Array.new(limit + 1){Array.new(v + 1, 0)}

  # Initial values alpha^1_{v,j}(1).
  a[1][1] = 1
  2.upto(v - 1){|j| a[1][j] = catalan(j - 1)}
  a[1][v] = catalan(v - 2)

  2.upto(limit){|d|
    # j = 1: compute u(d).
    s = a[d - 1][v]
    1.upto(d - 1){|i|
      s += a[i][v - 1] * a[d - i][1]
    }
    a[d][1] = s

    # j = 2, ..., v - 1: compute b(d), then c(d).
    2.upto(v - 1){|j|
      s = a[d][j - 1] * a[1][1]
      2.upto(j - 1){|g|
        s += a[d][j - g] * a[1][g]
      }
      1.upto(d - 1){|i|
        s += a[i][v - 1] * a[d - i][j]
      }
      a[d][j] = s
    }

    # j = v: compute s(d), i.e. d complete diamonds.
    s = a[d][v - 1]
    1.upto(d - 1){|i|
      s += a[i][v - 1] * a[d - i][v]
    }
    a[d][v] = s
  }

  values = [1]
  1.upto(limit){|d| values << a[d][v]}
  values
end

n = 20
p ary = a260332(n)