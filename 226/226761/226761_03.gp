M=25;

\\ a(n) = Sum_{k=0..n} (-3)^(n-k) * binomial(3*n+k,k).
a(n) = sum(k=0, n, (-3)^(n-k) * binomial(3*n+k,k));
for(n=0, M, print1(a(n)", "));