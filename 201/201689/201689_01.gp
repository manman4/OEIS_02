M=20;

\\ G.f. A(x) satisfies A(x) = 1 - x^2 * d/dx log(1 - x*A(x)).
my(A=1, n=M); for(i=1, n, A=1 - x^2*deriv(log(1 - x*A)) + x*O(x^M)); Vec(A)
