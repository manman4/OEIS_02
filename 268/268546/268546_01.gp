\\ a(n) = binomial(2*n,n) * Sum_{k=0..n} binomial(2*k,k) * binomial(3*n+k+1,2*k).
a(n) = binomial(2*n,n) * sum(k=0, n, binomial(2*k,k) * binomial(3*n+k+1,2*k));
for(n=0, 22, print1(a(n), ", ")); 