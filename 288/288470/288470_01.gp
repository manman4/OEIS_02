\\ a(n) = Sum_{k=0..floor(n/2)} binomial(2*n,k) * binomial(3*n-2*k-1,n-2*k). 
a(n) = sum(k=0, n\2, binomial(2*n, k)*binomial(3*n-2*k-1, n-2*k));
for(n=0, 20, print1(a(n), ", ")) 