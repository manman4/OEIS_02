\\ G.f.: Sum_{k>=0} x^(5*k) / ( (Product_{j=1..k} (1-x^(2*j))) * (Product_{j=1..2*k} (1-x^(2*j))) ).
my(N=60, x='x+O('x^N)); Vec(sum(k=0, N, x^(5*k)/(prod(j=1, k, (1-x^(2*j)))*prod(j=1, 2*k, (1-x^(2*j))))))
my(N=60, x='x+O('x^N)); sum(k=0, N, x^(5*k)/(prod(j=1, k, (1-x^(2*j)))*prod(j=1, 2*k, (1-x^(2*j)))))