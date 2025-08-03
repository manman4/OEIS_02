M=20;

\\ a(n) = Sum_{k=0..n} (-4)^(n-k) * binomial(4*n+1,k).
a(n) = sum(k=0, n, (-4)^(n-k) * binomial(4*n+1, k));
for(n=0, M, print1(a(n)", "));