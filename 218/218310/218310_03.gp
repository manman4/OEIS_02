\\ G.f.: 1/2 + 1/2 * Sum_{k>=0} (5*k+1)^(k-1) * x^k/(1 - (5*k+1)*x)^(k+1).
my(N=20, x='x+O('x^N)); Vec( 1/2 + 1/2 * sum(k=0, N, (5*k+1)^(k-1)*x^k/(1-(5*k+1)*x)^(k+1)) )

