\\ a(0) = 1; a(n) = n*a(n-1) - Sum_{k=1..floor(n/2)} (-1)^k * binomial(n,2*k) * a(n-2*k).
a(n) = if(n==0, 1, n*a(n-1) - sum(k=1, n\2, (-1)^k * binomial(n,2*k) * a(n-2*k)));
for(n=0, 20, print1(a(n),", "));