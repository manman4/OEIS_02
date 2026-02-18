\\ G.f.: Sum_{k>=0} x^(2*k) / ( (Product_{j=1..k} (1-x^j)) * (Product_{j=1..2*k} (1-x^j)) ).
my(N=60, x='x+O('x^N)); Vec(sum(k=0, N, x^(2*k)/(prod(j=1, k, (1-x^j))*prod(j=1, 2*k, (1-x^j)))))
my(N=60, x='x+O('x^N)); sum(k=0, N, x^(2*k)/(prod(j=1, k, (1-x^j))*prod(j=1, 2*k, (1-x^j))))