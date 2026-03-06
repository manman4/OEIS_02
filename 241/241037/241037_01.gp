\\ G.f.: Sum_{i>=1} Sum_{j>=1} q^(3*i+j) * Product_{k=i+1..2*i+j-1} (1+q^k).
my(N=60, q='q+O('q^N)); sum(i=1, N, sum(j=1, N, q^(3*i+j)*prod(k=i+1, 2*i+j-1, 1+q^k)))
my(N=60, q='q+O('q^N)); Vec(sum(i=1, N, sum(j=1, N, q^(3*i+j)*prod(k=i+1, 2*i+j-1, 1+q^k))))
