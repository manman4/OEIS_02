\\ a(n) = Sum_{k=0..n} (-2)^k * (3/2)^(n-k) * binomial(-5/2,k) * binomial(k,n-k).
a(n) = sum(k=0, n, (-2)^k * (3/2)^(n-k) * binomial(-5/2,k) * binomial(k,n-k));
for(n=0, 29, print1(a(n),", "))    