\\ G.f.: Sum_{j>=1} [z^j] ( Product_{k>=1} (1 + z*q^(j*k)/(1-q^k)) - Product_{k>=1} (1 + z*q^((j+1)*k)/(1-q^k)) ).
my(N=70, q='q+O('q^N)); sum(j=1, N, polcoef(prod(k=1, N, 1+z*q^(j*k)/(1-q^k)), j, z) - polcoef(prod(k=1, N, 1+z*q^((j+1)*k)/(1-q^k)), j, z))
my(N=70, q='q+O('q^N)); Vec(sum(j=1, N, polcoef(prod(k=1, N, 1+z*q^(j*k)/(1-q^k)), j, z) - polcoef(prod(k=1, N, 1+z*q^((j+1)*k)/(1-q^k)), j, z)))
