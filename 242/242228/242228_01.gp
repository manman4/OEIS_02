\\ E.g.f.: Sum_{k>=1} (exp(k^2*x) - 1)^k / k.
my(N=20, x='x+O('x^N)); Vec(serlaplace(sum(k=1, N, (exp(k^2*x) - 1)^k / k )))  