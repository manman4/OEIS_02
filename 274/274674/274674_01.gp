\\ a(n) = Sum_{k=0..floor(n/2)} binomial(n,2*k) * binomial(n+k,2*k) * binomial(2*k,k).
a(n) = sum(k=0, n\2, binomial(n, 2*k) * binomial(n+k, 2*k) * binomial(2*k, k));
for(n=0, 23, print1(a(n),", "));