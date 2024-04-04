\\ a(n) = Sum_{k=0..n} binomial(n,k) * binomial(n+3*k/2+1/2,n)/(2*n+3*k+1).
a(n) = sum(k=0, n, binomial(n, k) * binomial(n+3*k/2+1/2, n)/(2*n+3*k+1));
for(n=0, 20, print1(a(n), ", "))
