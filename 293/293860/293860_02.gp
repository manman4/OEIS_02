\\ a(n) = Sum_{k=1..n-1} n^k * binomial(n,k) * a(n-k) for n > 1.
a(n) = if(n<2, n, sum(k=1, n-1, n^k * binomial(n,k) * a(n-k)));                                                     
for(n=0, 19, print1(a(n),", ")); 