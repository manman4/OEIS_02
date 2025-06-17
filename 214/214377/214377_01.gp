\\ G.f. A(x) satisfies A(x) = 1 + 4*x*A(x)^(3/2).
my(A=1, n=22); for(i=1, n, A= 1 + 4 * x * A^(3/2) + x*O(x^n) ); Vec(A)

\\ G.f. A(x) satisfies A(x) = 1/A(-x*A(x)^2).
my(A=1, n=22); for(i=1, n, A= 1 + 4 * x * A^(3/2) + x*O(x^n) ); Vec(1/subst(A, x, -x*A^2))


