\\ a293145(n) = n! * Sum_{k=1..n} n^k     * binomial(n-1,k-1)/k! for n > 0.
\\ a293146(n) = n! * Sum_{k=1..n} n^(n-k) * binomial(n-1,k-1)/k! for n > 0.

a(n) = if(n==0, 1, n!*sum(k=1, n, n^(n-k)*binomial(n-1, k-1)/k!));
for(n=0, 20, print1(a(n), ", "))