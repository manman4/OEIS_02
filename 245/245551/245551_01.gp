\\ a(n) = (binomial(n+4,2)/6) * Sum_{k=0..floor(n/2)} binomial(n+2,n-2*k) * binomial(2*k+2,k).
a(n) = (binomial(n+4,2)/6) * sum(k=0, n\2, binomial(n+2,n-2*k) * binomial(2*k+2,k));
for(n=0, 29, print1(a(n),", "))    