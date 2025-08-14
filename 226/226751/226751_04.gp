\\ a(n) = Sum_{k=0..n} (-2)^k * 3^(n-k) * binomial(3*n+1,k) * binomial(3*n-k,n-k).
a(n) = sum(k=0, n, (-2)^k * 3^(n-k) * binomial(3*n+1,k) * binomial(3*n-k,n-k));
for(n=0, 20, print1(a(n),", "))