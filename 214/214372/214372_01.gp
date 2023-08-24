\\ a(n+1) = Sum_{k=0..n} binomial(n+k+1,k) * binomial(2*k,n-k)/(n+k+1).
a(n) = sum(k=0, n-1, binomial(n+k, k) * binomial(2*k, n-1-k)/(n+k));
for(n=1, 20, print1(a(n), ", "))     
