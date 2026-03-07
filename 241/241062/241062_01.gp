\\ G.f.: Sum_{j>=1} q^(3*j+1) * Product_{k=j+1..2*j} (1+q^k). 
my(N=90, q='q+O('q^N)); sum(j=1, N, q^(3*j+1) * prod(k=j+1, 2*j, 1+q^k))
my(N=90, q='q+O('q^N)); Vec(sum(j=1, N, q^(3*j+1) * prod(k=j+1, 2*j, 1+q^k)))
