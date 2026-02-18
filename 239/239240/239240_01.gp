\\ G.f.: Sum_{i>=0} Sum_{j=0..i} x^(i*(i+1)+j^2) / ( (Product_{k=1..i} (1-x^(2*k))) * (Product_{k=1..j} (1-x^(2*k))) ). 
my(N=60, x='x+O('x^N)); Vec(sum(i=0, N, sum(j=0, i, x^(i*(i+1)+j^2)/(prod(k=1, i, (1-x^(2*k)))*prod(k=1, j, (1-x^(2*k)))))))
my(N=60, x='x+O('x^N)); sum(i=0, N, sum(j=0, i, x^(i*(i+1)+j^2)/(prod(k=1, i, (1-x^(2*k)))*prod(k=1, j, (1-x^(2*k))))))
