\\ a(n) = Sum_{k=0..n} binomial(n,k) * binomial(2*n+2*k+1,n)/(2*n+2*k+1).
a(n) = sum(k=0, n, binomial(n, k)*binomial(2*n+2*k+1, n)/(2*n+2*k+1));
for(n=0, 20, print1(a(n),", ")) 


