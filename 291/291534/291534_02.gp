a(n) = sum(k=0, n, (-1)^k * 2^(n-k) * binomial(n,k) * binomial(2*n+k+1,n) / (2*n+k+1) ); 
for(n=1, 19, print1(a(n),", "))   
