\\ G.f.: Sum_{j>=1} q^(6*j) * Product_{k=j+1..5*j-1} (1+q^k).
my(N=100, q='q+O('q^N)); sum(j=1, N, q^(6*j)*prod(k=j+1, 5*j-1, 1+q^k))
my(N=100, q='q+O('q^N)); Vec(sum(j=1, N, q^(6*j)*prod(k=j+1, 5*j-1, 1+q^k)))
