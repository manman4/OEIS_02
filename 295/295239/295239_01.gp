a000108(n) = binomial(2*n, n)/(n+1);

\\ a(n) = n! * Sum_{k=0..n} (-1)^k * k^(n-k) * A000108(k)/(n-k)!.
a(n) = n! * sum(k=0, n, (-1)^k * k^(n-k) * a000108(k)/(n-k)!);                        
for(n=0, 19, print1(a(n),", "))   