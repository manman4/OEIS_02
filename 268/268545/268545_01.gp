\\ a(n) = Sum_{k = 0..n} C(2*n, n-k) * C(2*n+k, k).
a026000(n) = sum(k=0, n, binomial(2*n, n-k) * binomial(2*n+k, k));
\\ a(n) = binomial(2*n,n) * A026000(n).
a(n) = binomial(2*n, n) * a026000(n);
for(n=0, 22, print1(a(n), ", ")); 