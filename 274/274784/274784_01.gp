\\ a(n) = Sum_{k=0..floor(n/2)} binomial(n,2*k) * binomial(n+k,k) * binomial(2*k,k)^2.
a(n) = sum(k=0, n\2, binomial(n, 2*k) * binomial(n+k, k) * binomial(2*k, k)^2);
for(n=0, 20, print1(a(n),", "));