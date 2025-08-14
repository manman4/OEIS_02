\\ G.f.: G(x)^2/((-3+4*G(x)) * (4-3*G(x))) where G(x) = 1+x*G(x)^4 is the g.f. of A002293. 
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(4*k, k)/(3*k+1)*x^k)); Vec( g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(4*k, k)/(3*k+1)*x^k)); Vec( 1+x*g^4 - g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(4*k, k)/(3*k+1)*x^k)); Vec( g^2/((-3+4*g) * (4-3*g)) )