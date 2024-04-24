\\ a(n) = 3/2 * Sum_{k=0..n} (2*k+3)^(n-1) * binomial(n,k) for n > 0.
a(n) = if(n==0, 1, 3/2*sum(k=0, n, (2*k+3)^(n-1)*binomial(n, k)));
for(n=0, 16, print1(a(n), ", "))

