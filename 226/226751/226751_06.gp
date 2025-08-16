\\ G.f.: B(x)^2/(1 + 5*(B(x)-1)/3), where B(x) is the g.f. of A005809. 
my(N=30, x='x+O('x^N), b=sum(k=0, N, binomial(3*k, k)*x^k)); Vec( b )
my(N=30, x='x+O('x^N), b=sum(k=0, N, binomial(3*k, k)*x^k)); Vec( b^2/(1 + 5*(b-1)/3) )