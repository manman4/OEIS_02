\\ G.f.: Sum_{i>=0} x^(i*(2*i+1)) / Product_{k=1..i} (1-x^(2*k))^2.
my(N=60, x='x+O('x^N)); Vec(sum(i=0, N, x^(i*(2*i+1))/prod(k=1, i, (1-x^(2*k))^2) ))

