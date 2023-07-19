a(n) = sum(k=0, n, binomial(2*n+2*k+1,k) * binomial(2*n+2*k+1,n-k) / (2*n+2*k+1) );

for(n=0, 20, print1(a(n),", "))  

