\\ G.f.: Sum_{i>=0} [z^i] Product_{j>=1} (1 + z * Sum_{k=1..i} q^(j*k)).
my(N=50, q='q+O('q^N)); Vec(sum(i=0, N, polcoef(prod(j=1, N, 1+z*sum(k=1, i, q^(j*k))), i, z)))
