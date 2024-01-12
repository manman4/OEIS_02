\\ a(n+1) = (1/(n+1)) * Sum_{k=0..n} binomial(2*n+k+1,k) * binomial(n-1,n-k).
a(n) = 1/n * sum(k=0, n-1, binomial(2*n+k-1, k) * binomial(n-2, n-k-1));
for(n=1, 20, print1(a(n), ", ")); 