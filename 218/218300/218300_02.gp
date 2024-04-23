\\ a(n) = Sum_{k=0..n} (k+2)^(n-1) * binomial(n,k) for n > 0.
a(n) = if(n==0, 1, sum(k=0, n, (k+2)^(n-1)*binomial(n, k)));
for(n=0, 20, print1(a(n), ", "))

