\\ G.f.: G(x)^2/((-2+3*G(x)) * (3-2*G(x))) where G(x) = 1+x*G(x)^3 is the g.f. of A001764. 
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( 1+x*g^3 - g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( g^2/((-2+3*g) * (3-2*g)) )