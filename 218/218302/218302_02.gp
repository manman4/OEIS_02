\\ a(n) = 2 * Sum_{k=0..n} (k+4)^(n-1) * binomial(n,k) for n > 0.
a(n) = if(n==0, 1, 2*sum(k=0, n, (k+4)^(n-1)*binomial(n, k)));
for(n=0, 20, print1(a(n), ", "))

