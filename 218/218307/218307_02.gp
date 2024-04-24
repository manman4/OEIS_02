\\ a(n) = 1/2 * Sum_{k=0..n} (4*k+1)^(n-1) * binomial(n,k) for n > 0.
a(n) = if(n==0, 1, 1/2*sum(k=0, n, (4*k+1)^(n-1)*binomial(n, k)));
for(n=0, 20, print1(a(n), ", "))

