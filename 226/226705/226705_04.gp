M=25;

\\ a(n) = Sum_{k=0..n} (-2)^k * 3^(n-k) * binomial(6*n+1,k) * binomial(6*n-k,n-k).
a(n) = sum(k=0, n, (-2)^k * 3^(n-k) * binomial(6*n+1,k) * binomial(6*n-k,n-k));
for(n=0, M, print1(a(n)", "));

