\\ E.g.f.: Sum_{k>=0} (k+1)^2 * log(1+x)^k / k!.
my(N=30, x='x+O('x^N)); Vec(serlaplace(sum(k=0, N, (k+1)^2 * log(1+x)^k/k!)))

