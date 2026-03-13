\\ G.f.: Sum_{i>=1} [z^i] ( Product_{j>=1} (1 + z * Sum_{k=1..i} q^(j*k)) - Product_{j>=1} (1 + z * Sum_{k=1..i-1} q^(j*k)) ).
my(N=60, q='q+O('q^N)); sum(i=1, N, polcoef(prod(j=1, N, 1+z*sum(k=1, i, q^(j*k))), i, z) - polcoef(prod(j=1, N, 1+z*sum(k=1, i-1, q^(j*k))), i, z))
my(N=60, q='q+O('q^N)); Vec(sum(i=1, N, polcoef(prod(j=1, N, 1+z*sum(k=1, i, q^(j*k))), i, z) - polcoef(prod(j=1, N, 1+z*sum(k=1, i-1, q^(j*k))), i, z)))
