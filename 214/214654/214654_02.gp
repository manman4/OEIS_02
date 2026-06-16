\\ E.g.f. A(x) satisfies A'(x) = exp( A(x)*A'(x) ), with A(0)=0.
my(N=20, x='x+O('x^(N+1)), A=x+O('x^(N+1))); for(i=1, N, A=intformal(exp(A*deriv(A)))+O('x^(N+1))); vector(N, n, n!*polcoeff(A,n))
