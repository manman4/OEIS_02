\\ E.g.f.: sqrt(1-x^2)*exp(x*arctanh(x)).
my(N=30, x='x+O('x^N)); Vec(serlaplace(sqrt(1-x^2) * exp(x*atanh(x))))

\\ E.g.f. A(x) satisfies A'(x)/A(x) = arctanh(x). 
my(N=30, x='x+O('x^N), A=sqrt(1-x^2) * exp(x*atanh(x))); Vec(serlaplace(deriv(A)/A - atanh(x)))