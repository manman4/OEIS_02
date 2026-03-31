\\ Expansion of e.g.f. sqrt(1-x^2) * exp(x*(1+arctanh(x))).
my(N=30, x='x+O('x^N)); Vec(serlaplace(sqrt(1-x^2) * exp(x*(1+atanh(x)))))

\\ E.g.f. A(x) satisfies A'(x)/A(x) = 1 + arctanh(x).
my(N=30, x='x+O('x^N), A=sqrt(1-x^2) * exp(x*(1+atanh(x)))); Vec(serlaplace(deriv(A)/A - (1 + atanh(x))))