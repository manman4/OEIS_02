\\ a(n+1) = (1/(n+1)) * Sum_{k=0..floor(n/2)} binomial(n+k,k) * binomial(3*n-k+1,n-2*k).
a(n) = sum(k=0, (n-1)\2, binomial(n+k-1, k)*binomial(3*n-k-2, n-2*k-1))/n; 
for(n=1, 30, print1(a(n), ", "));  