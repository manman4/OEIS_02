\\ G.f.: Sum_{j>=1} q^(5*j) * Product_{k=j+1..4*j-1} (1+q^k). 
my(N=100, q='q+O('q^N)); sum(j=1, N, q^(5*j)*prod(k=j+1, 4*j-1, 1+q^k))
my(N=100, q='q+O('q^N)); Vec(sum(j=1, N, q^(5*j)*prod(k=j+1, 4*j-1, 1+q^k)))
