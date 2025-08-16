\\ G.f.: B(x)^2/(1 + 3*(B(x)-1)/2), where B(x) is the g.f. of A005810. 
my(N=30, x='x+O('x^N), b=sum(k=0, N, binomial(4*k, k)*x^k)); Vec( b )
my(N=30, x='x+O('x^N), b=sum(k=0, N, binomial(4*k, k)*x^k)); Vec( b^2/(1 + 3*(b-1)/2) )