\\ G.f.: 1 + Sum_{j>=1} q^j*(1-q^(j^2))/(1-q^j) * Product_{k=1..j-1} (1-q^((j+1)*k))/(1-q^k). 
my(N=60, q='q+O('q^N)); Vec(1 + sum(j=1, N, q^j*(1-q^(j^2))/(1-q^j)*prod(k=1, j-1, (1-q^((j+1)*k))/(1-q^k))))
