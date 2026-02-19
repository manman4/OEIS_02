\\ G.f.: Sum_{k>=0} x^(3*k+10) / ( (Product_{j=1..k} (1-x^(2*j))) * (Product_{j=1..k+10} (1-x^(2*j))) ).
my(N=60, x='x+O('x^N)); Vec(sum(k=0, N, x^(3*k+10)/(prod(j=1, k, (1-x^(2*j)))*prod(j=1, k+10, (1-x^(2*j))))))
my(N=60, x='x+O('x^N)); sum(k=0, N, x^(3*k+10)/(prod(j=1, k, (1-x^(2*j)))*prod(j=1, k+10, (1-x^(2*j)))))